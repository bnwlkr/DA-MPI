#include "profile.h"

static struct ProcInfo info; // global because native to process, not rank (doesn't move in migration)

/* calculate offset of this proc's block in edge (frequency and delay) tables */
static int offset (int a) {
  return (info.n-1)*a - (a-1)*a/2;
}

/* determine which node is best suited to holding frequency and translation tables (best-connected node) */
static int best () {
  int best = 0;
  double best_delay = 0.0;
  for (int i = 0; i < info.n; i++) {
    double delay = 0.0;
    int offset_ = offset(i);
    int n_read = info.n-i-1;
    for (int j = offset_; j < offset_ + n_read; j++) {
      delay += info.delays[j];
    }
    for (int j = 0; j < i; j++) {
      int offset_ = offset(j);
      delay += info.delays[offset_+i-j-1];
    }
    if (delay < best_delay || best_delay == 0.0) {
      best_delay = delay;
      best = i;
    }
  }
  return best;
}


/* free everything that MPIX_Profile built for this proc */
static void destroy () {
  free(info.delays);
  free(info.wbase);
  MPI_Win_free(&info.win);
}

/* respond to requests for latency measurement from other procs */
static void respond (char* data) {
  MPI_Status status;
  int done = 0;
  while (!done) {
    MPI_Recv(data, DATA_SIZE, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch (status.MPI_TAG) {
      case REQUEST:
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, status.MPI_SOURCE, RESPONSE, MPI_COMM_WORLD);
        break;
      default:
        return;
    }
  }
}

/* measure latencies with other procs */
static void measure (char* data) {
  int offset_ = offset(info.proc);
  for (int i = info.proc+1; i < info.n; i++) {
    double t0 = MPI_Wtime();
    for (int j = 0; j < TRIALS; j++) {
      MPI_Ssend(data, DATA_SIZE, MPI_BYTE, i, REQUEST, MPI_COMM_WORLD);
      MPI_Recv(data, DATA_SIZE, MPI_BYTE, i, RESPONSE, MPI_COMM_WORLD, NULL);
    }
    double t1 = MPI_Wtime();
    info.delays[offset_ + i-(info.proc+1)] = t1-t0;
  }
}


void MPIX_Profile (int proc, int n) {
  info.proc = proc;
  info.n = n;
  int n_edges = n*(n-1)/2;
  info.delays = calloc(n_edges, sizeof(double));
  MPI_Win win;
  MPI_Win_create(info.delays, n_edges*sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
  char data[DATA_SIZE];
  switch (proc) {
    case 0:
      measure(data);
      MPI_Ssend(data, DATA_SIZE, MPI_BYTE, 1, NEXT, MPI_COMM_WORLD);
      break;
    default:
      respond(data);
      measure(data);
      if (proc < n-1) {
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, proc+1, NEXT, MPI_COMM_WORLD);
      }
  }
  MPI_Win_fence(0, win);
  for (int i = 0; i < n; i++) {
    if (i != proc) {
     int n_read = n-i-1;
     int offset_= offset(i);
     MPI_Get(&info.delays[offset_], n_read, MPI_DOUBLE, i, offset_, n_read, MPI_DOUBLE, win);
    }
  }
  for (int i = 0; i < n_edges; i++) {
    printf("proc %d has %f at %d\n", info.proc, info.delays[i], i);
  }
  MPI_Win_fence(0, win);
  MPI_Win_free(&win);
  
  info.bnode = best();  
  
  /* ===================== */
  info.wbase = calloc(n_edges, sizeof(int));
  info.wbase[0] = 10;
  info.wbase[1] = 30;
  info.wbase[2] = 50;
  MPI_Win w;
  MPI_Win_create(info.wbase, n_edges*sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &w);
  MPI_Win_fence(0, w);
  
  int r[n_edges];
  r[0] = -1;
  r[1] = -1;
  r[2] = -1;
  int main = 2;
  if (proc != main) {
    MPI_Get(r, n_edges, MPI_INT, main, 0, 1, MPI_INT, w);
    for (int i = 0; i < n_edges; i++) {
      printf("proc %d found %d in %d\n", info.proc, r[i], main);
    }
  }
  
  
  
  MPI_Win_fence(0, w);
  

  
  
  
  
  
  
  
  
  //destroy();
  
  
}







