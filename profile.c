#include "profile.h"

static struct Info info;

static int offset (int a) {
  return (info.n-1)*a - (a-1)*a/2;
}

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

static void destroy () {
  free(info.delays);
  free(info.wbase);
  MPI_Win_free(&info.win);
}

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

int count (int a, int b) {
  int min = MIN(a,b);
  int max = MAX(a,b);
  int offset_ = offset(min)+max-1;
  int result;
  printf("%d getting frequency from %d\n", info.proc, info.bnode);
  MPI_Get(&result, 1, MPI_INT, info.bnode, offset_, 1, MPI_INT, info.win);
  return result;
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
  MPI_Win_fence(0, win);
  MPI_Win_free(&win);
  
  info.bnode = best();  
  int best = proc == info.bnode;

  info.wbase = calloc(sizeof(int), best ? n_edges : 0);
    
  MPI_Win_create(info.wbase, best ? n_edges*sizeof(int) : 0, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &info.win);
  
  /* CURRENT ISSUE: CHANING LOCAL MEMORY DOESNT SEEM TO UPDATE WINDOW FOR RMA */
  

  printf("%d\n", info.bnode);
  MPI_Win_fence(0, info.win);
  printf("proc %d, %d--%d = %d\n", proc, 0,1,count(0,1));
  MPI_Win_fence(0, info.win);
  
  destroy();
  
  
}







