#include "migrate.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static void get_bt (struct BNodeTable* bt) {
  int read = sizeof(struct BNodeTable)/sizeof(int) + info->n_edges;
  MPI_Get(bt, read, MPI_INT, info->bnode, 0, read, MPI_INT, info->bwin);
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

static void swap_cases(int a, int b) {
//  int a_size = info->rank_sc_sizes[info->bt->a];
//  int b_size = info->rank_sc_sizes[info->bt->b];
//  int read = info->rank==info->bt->a ? b_size : a_size;
//  int other = info->rank==info->bt->a ? info->bt->b : info->bt->a;
//  void* temp = malloc(read);
//  MPI_Win_lock(MPI_LOCK_SHARED, other, 0, exwin);
//  printf("%d attemtping to swap case with %d\n", info->rank, other);
//  MPI_Get(temp, read, MPI_BYTE, other, 0, read, MPI_BYTE, exwin);
//  printf("MPI GOT\n");
//  MPI_Win_unlock(other, exwin);
//  free(info->suitcase);
//  info->suitcase = malloc(read);
//  memcpy(info->suitcase, temp, read);
}


void DAMPI_Airlock () {
  MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->bwin);
  get_bt(info->bt);
  if (info->bt->a == -1) {
    info->bt->b = should_migrate(info->bt);
    if (info->bt->b != -1) {
      printf ("SWAP REQUEST: %d <--> %d\n", info->rank, info->bt->b);
      info->bt->a = info->rank;
      MPI_Put(info->bt, 2, MPI_INT, info->bnode, 0, 2, MPI_INT, info->bwin);
      goto migration;
    } else {
      MPI_Win_unlock(info->bnode, info->bwin);
    }
  } else {
    migration:
    MPI_Win_unlock(info->bnode, info->bwin);
    swap_rankproc_info();
    int a = info->bt->a;
    int b = info->bt->b;
    int part = info->rank == a || info->rank == b;
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->bwin);
    info->bt->a = info->bt->b = -1;
    MPI_Win_unlock(info->bnode, info->bwin);
    if (part) {
      printf("SWAP: %d <--> %d\n", info->rank, info->rank == a ? b : a);
      info->rank = b;
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
}









