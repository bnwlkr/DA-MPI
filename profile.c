#include "profile.h"

struct ProcInfo info; // global because native to process, not rank (doesn't move in migration)

/* calculate offset of this proc's block in edge (frequency and delay) tables */
int boffset (int a) {
  return (info.n-1)*a - (a-1)*a/2;
}

int DAMPI_Eoffset (int a, int b) {
  int min = MIN(a,b);
  int max = MAX(a,b);
  int boffset_ = boffset(min);
  return boffset_+max-min-1;
}

/* determine which node is best suited to holding frequency and translation tables (best-connected node) */
static int best () {
  int best = 0;
  double best_delay = 0.0;
  for (int i = 0; i < info.n; i++) {
    double delay = 0.0;
    int offset_ = boffset(i);
    int n_read = info.n-i-1;
    for (int j = offset_; j < offset_ + n_read; j++) {
      delay += info.delays[j];
    }
    for (int j = 0; j < i; j++) {
      int offset_ = boffset(j);
      delay += info.delays[offset_+i-j-1];
    }
    if (delay < best_delay || best_delay == 0.0) {
      best_delay = delay;
      best = i;
    }
  }
  return best;
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
  int offset_ = boffset(info.proc);
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


void DAMPI_Finalize () {
  free(info.delays);
  MPI_Win_free(&info.win);
}

void DAMPI_Profile (int proc, int n) {
  info.proc = proc;
  info.n = n;
  info.n_edges = n*(n-1)/2;
  info.delays = calloc(info.n_edges, sizeof(double));
  MPI_Win win;
  MPI_Win_create(info.delays, info.n_edges*sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
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
     int offset_= boffset(i);
     MPI_Get(&info.delays[offset_], n_read, MPI_DOUBLE, i, offset_, n_read, MPI_DOUBLE, win);
    }
  }
  
  MPI_Win_fence(0, win);
  MPI_Win_free(&win);

  info.bnode = best();  
  MPI_Win_allocate(proc == info.bnode ? info.n_edges*sizeof(int) : 0, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &info.wbase, &info.win);
  memset(info.wbase, 0, proc == info.bnode ? info.n_edges : 0);
}


void DAMPI_Info_sync() {
  MPI_Win_fence(0, info.win);
}


void DAMPI_Diag() {
  if (info.proc == info.bnode) {
    printf("n: %d, n_edges: %d, bnode: %d\n", info.n, info.n_edges, info.bnode);
    for (int i = 0; i < info.n_edges; i++) {
      printf("freq[%d] = %d\n", i, info.wbase[i]);
    }
    for (int i = 0; i < info.n_edges; i++) {
      printf ("delays[%d] = %f\n", i, info.delays[i]);
    }
  }
}



//  MPI_Win_fence(0, info.win);
//  
//  int freqs[n_edges];
//  
//  MPI_Get(freqs, n_edges, MPI_INT, info.bnode, 0, n_edges, MPI_INT, info.win);
//  
//  for (int i=0; i<n_edges; i++) {
//    printf("freqs[%d] = %d\n", i, freqs[i]);
//  }
//  
//  MPI_Win_fence(0, info.win);




