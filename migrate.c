#include "migrate.h"
#include <string.h>
#include <stdio.h>


void get_bt (struct BNodeTable* bt) {
  MPI_Win_lock(MPI_LOCK_SHARED, info->bnode, 0, info->bwin);
  MPI_Get(bt, sizeof(struct BNodeTable)/sizeof(int) + info->n_edges, MPI_INT, info->bnode, 0, sizeof(struct BNodeTable)/sizeof(int) + info->n_edges*2, MPI_INT, info->bwin);
  MPI_Win_unlock(info->bnode, info->bwin);
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
    printf("%d should swap with %d. value difference: %f\n", info->rank, highest_swap, highest_val - current_val);
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










