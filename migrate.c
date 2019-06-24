#include "migrate.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static void get_bt (struct BNodeTable* bt) {
  MPI_Get(bt, 2, MPI_INT, info->bnode, 0, 2, MPI_INT, info->bwin);
  MPI_Get(bt->freq, info->n_edges, MPI_INT, info->bnode, 0, info->n_edges, MPI_INT, info->freqwin);
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
    printf("%f\n", highest_val-current_val);
    return highest_swap;
  }
  return -1;
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


static void swap_rankproc_info () {
  int temp = info->rankprocs[info->bt->a];
  info->rankprocs[info->bt->a] = info->rankprocs[info->bt->b];
  info->rankprocs[info->bt->b] = temp;
}

static void swap(int a, int b) {
  char dummy;
  int other_rank = info->rank==a ? b : a;
  int other_proc = info->rankprocs[other_rank];
  void* temp = malloc(info->sc_size);
  MPI_Sendrecv(info->suitcase, info->sc_size, MPI_BYTE, other_proc, 0, temp, info->sc_size, MPI_BYTE, other_proc, 0, MPI_COMM_WORLD, NULL);
  memcpy(info->suitcase, temp, info->sc_size);
  free(temp);
  printf("SWAP %d <--> %d\n", info->rank, other_rank);
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

void DAMPI_Airlock () {
  DAMPI_Diag();
  LOCKBN();
  get_bt(info->bt);
  if (info->bt->a == -1) {
    info->bt->b = should_migrate(info->bt);
    if (info->bt->b != -1) {
      printf ("SWAP REQUEST: %d <--> %d\n", info->rank, info->bt->b);
      info->bt->a = info->rank;
      MPI_Put(info->bt, 2, MPI_INT, info->bnode, 0, 2, MPI_INT, info->bwin);
      goto migration;
    } else {
      UNLOCKBN();
    }
  } else {
    migration: UNLOCKBN();
    int a = info->bt->a;
    int b = info->bt->b;
    int part = info->rank == a || info->rank == b;
    if (part) {
      swap(a,b);
    }
    swap_rankproc_info();
    MPI_Win_fence(0, info->bwin);
    info->bt->a = info->bt->b = -1;
    MPI_Put(info->bt, 2, MPI_INT, info->bnode, 0, 2, MPI_INT, info->bwin);
    MPI_Win_fence(0, info->bwin);
  }
}









