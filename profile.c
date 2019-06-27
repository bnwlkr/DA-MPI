#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "profile.h"
#include "migrate.h"
#include "dampi.h"

struct ProcInfo * info; // global because native to process, not rank (doesn't move in migration)

/* calculate offset of this proc's block in edge (frequency and delay) tables */
int boffset (int a) {
  return (info->n-1)*a - (a-1)*a/2;
}

int eoffset (int a, int b) {
  int min = MIN(a,b);
  int max = MAX(a,b);
  int boffset_ = boffset(min);
  return boffset_+max-min-1;
}

/* determine which node is best suited to holding frequency and translation tables (best-connected node) */
static int best () {
  int best = 0;
  double best_delay = 0.0;
  for (int i = 0; i < info->n; i++) {
    double delay = 0.0;
    int offset_ = boffset(i);
    int n_read = info->n-i-1;
    for (int j = offset_; j < offset_ + n_read; j++) {
      delay += info->delays[j];
    }
    for (int j = 0; j < i; j++) {
      int offset_ = boffset(j);
      delay += info->delays[offset_+i-j-1];
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
  int offset_ = boffset(info->proc);
  MPI_Status status;    // for mpich
  for (int i = info->proc+1; i < info->n; i++) {
    double t0 = MPI_Wtime();
    for (int j = 0; j < TRIALS; j++) {
      MPI_Ssend(data, DATA_SIZE, MPI_BYTE, i, REQUEST, MPI_COMM_WORLD);
      MPI_Recv(data, DATA_SIZE, MPI_BYTE, i, RESPONSE, MPI_COMM_WORLD, &status);
    }
    double t1 = MPI_Wtime();
    info->delays[offset_ + i-(info->proc+1)] = t1-t0;
  }
}


void DAMPI_Finalize () {
  MPI_Win_free(&info->bwin);
  MPI_Win_free(&info->scwin);
  MPI_Win_free(&info->freqwin);
  free(info->rankprocs);
  free(info->rankfuncs);
  free(info->delays);
  free(info->sk);
  free(info);
}

static void sync_delays (MPI_Win* delay_win) {
  MPI_Win_fence(0, *delay_win);
  MPI_Put(&info->delays[boffset(info->proc)], info->n-info->proc-1, MPI_DOUBLE, 0, boffset(info->proc), info->n-info->proc-1, MPI_DOUBLE, *delay_win);
  MPI_Win_fence(0, *delay_win);
  MPI_Win_fence(0, *delay_win);
  MPI_Get(info->delays, info->n_edges, MPI_DOUBLE, 0, 0, info->n_edges, MPI_DOUBLE, *delay_win);
  MPI_Win_fence(0, *delay_win);
}


void profile (int proc, int n) {
  info = malloc(sizeof(struct ProcInfo));
  info->rankprocs = malloc(sizeof(int)*n);
  info->rankfuncs = malloc(sizeof(dampi_func)*n);
  info->sk = malloc(sizeof(struct SwapKit));
  info->delays = calloc(info->n_edges, sizeof(double));
  info->proc = proc;
  info->rank = proc;
  info->n = n;
  info->n_edges = n*(n-1)/2;
  MPI_Win delay_win;
  MPI_Win_create(info->delays, info->proc==0 ? info->n_edges*sizeof(double) : 0, sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &delay_win);
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
  sync_delays(&delay_win);
  info->bnode = best();  
  MPI_Win_allocate(sizeof(struct BNodeTable), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &info->bt, &info->bwin);
  MPI_Win_allocate(info->n_edges*sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &info->bt->freq, &info->freqwin);
  memset(info->bt->freq, 0, info->n_edges*sizeof(int));
  info->bt->a = info->bt->b = -1;
  for (int i = 0; i < info->n; i++) {
    info->rankprocs[i] = i;
  }
}


void DAMPI_Diag() {
  if (info->proc == info->bnode) {
    printf("suitcase size: %d\n", info->sc_size);
    printf("n: %d, n_edges: %d, bnode: %d\n", info->n, info->n_edges, info->bnode);
    printf("a: %d, b: %d\n", info->bt->a, info->bt->b);
    for (int i = 0; i < info->n_edges; i++) {
      printf ("delays[%d] = %f\n", i, info->delays[i]);
    }
    for (int i = 0; i < info->n_edges; i++) {
      printf("freq[%d] = %d\n", i, info->bt->freq[i]);
    }
    for (int i = 0; i < info->n; i++) {
      printf("%d : [proc: %d, func: %p]\n", i, info->rankprocs[i], (void*)info->rankfuncs[i]);
    }
  }
}
