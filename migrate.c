#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "migrate.h"

int counter = 0;

static void get_bt (struct BNodeTable* bt) {
  if (info->proc != info->bnode) {
    MPI_Get(bt, 3, MPI_INT, info->bnode, 0, 3, MPI_INT, info->bwin);
    MPI_Get(bt->freq, info->n_edges, MPI_INT, info->bnode, 0, info->n_edges, MPI_INT, info->freqwin);
  }
}

double value (struct BNodeTable* bt, int* rankprocs) {
  double sum = 0.0;
  for (int i = 0; i < info->n-1; i++) {
    for (int j = i+1; j < info->n; j++) {
      int offset = eoffset(i,j);
      int freq = bt->freq[offset];
      int iproc = rankprocs[i];
      int jproc = rankprocs[j];
      double delay = info->delays[eoffset(iproc,jproc)];
      sum += (double)freq/delay;
    }
  }
  return sum;
}

int should_migrate (struct BNodeTable* bt) {
  double current_val = value(bt, info->rankprocs);
  double highest_val = 0.0;
  int highest_swap = 0;
  int rankprocs[info->n];
  memcpy(rankprocs, info->rankprocs, info->n*sizeof(int));
  for (int i = 0; i < info->n; i++) {
    if (i != info->rank) {
      int temp = rankprocs[i];
      rankprocs[i] = info->proc;
      rankprocs[info->rank] = temp;
      double val = value(bt, rankprocs);
      if (val > highest_val) {
        highest_val = val;
        highest_swap = i;
      }
      rankprocs[info->rank] = info->proc;
      rankprocs[i] = temp;
    }
  }
  if (highest_val - current_val > SWAP_THRESHOLD) {
    return highest_swap;
  }
  return -1;
}

static void swap_rankproc_info (int a, int b) {
  int temp = info->rankprocs[a];
  info->rankprocs[a] = info->rankprocs[b];
  info->rankprocs[b] = temp;
}

static void swap(int a, int b) {
  char dummy;
  int other_rank = info->rank==a ? b : a;
  int other_proc = info->rankprocs[other_rank];
  void* temp = malloc(info->sc_size);
  MPI_Status status;
  MPI_Sendrecv(info->suitcase, info->sc_size, MPI_BYTE, other_proc, 0, temp, info->sc_size, MPI_BYTE, other_proc, 0, MPI_COMM_WORLD, &status);
  memcpy(info->suitcase, temp, info->sc_size);
  free(temp);
  info->rank = other_rank;
}

static void LOCKBN () {
  MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->bwin);
  MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->freqwin);
}

static void UNLOCKBN () {
  MPI_Win_unlock(info->bnode, info->freqwin);
  MPI_Win_unlock(info->bnode, info->bwin);
}

int DAMPI_Airlock (int migrate) {
  LOCKBN();
  if (!migrate) {
    info->bt->valid = 0;
    if (info->proc != info->bnode) {
      MPI_Put(&info->bt->valid, 1, MPI_INT, info->bnode, 2, 1, MPI_INT, info->bwin);
    }
  } else {
    get_bt(info->bt);
    if (info->bt->valid) {
      if (info->bt->a == -1) {
        info->bt->b = should_migrate(info->bt);
        if (info->bt->b != -1) {
          info->bt->a = info->rank;
          if (info->proc != info->bnode) {
            MPI_Put(info->bt, 2, MPI_INT, info->bnode, 0, 2, MPI_INT, info->bwin);
          }
        }
      }
    }
  }
  UNLOCKBN();
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(info->bt, 3, MPI_INT, info->bnode, MPI_COMM_WORLD);
  if (info->bt->a == -1 || !info->bt->valid) {
    info->bt->a = info->bt->b = -1;
    info->bt->valid = 1;
    MPI_Barrier(MPI_COMM_WORLD);
    return 1;
  }
  int a = info->bt->a;
  int b = info->bt->b;
  int part = info->rank == a || info->rank == b;
  if (part) {
    swap(a,b);
  }
  swap_rankproc_info(a,b);
  info->bt->a = info->bt->b = -1;
  MPI_Barrier(MPI_COMM_WORLD);
  if (part) {
    info->rankfuncs[info->rank](info->suitcase);
    return 0;
  }
  return 1;
}









