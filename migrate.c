#include "migrate.h"
#include <string.h>
#include <stdio.h>



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


void DAMPI_Airlock () {
  printf("rank %d has entered the airlock\n", info->rank);
  struct BNodeTable bt;
  MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->bwin);
  get_bt(&bt);
  if (bt.a == -1) {
    bt.b = should_migrate(&bt);
    if (bt.b != -1) {
      printf ("rank %d wants to swap with %d\n", info->rank, bt.b);
      bt.a = info->rank;
      MPI_Put(&bt, 2, MPI_INT, info->bnode, 0, 2, MPI_INT, info->bwin);
      goto migration;
    } else {
      MPI_Win_unlock(info->bnode, info->bwin);
    }
  } else {
    migration: 
    if (bt.checkin == info->n-1) {
      bt.a = bt.b = -1;
      bt.checkin = 0;
      MPI_Put(&bt, sizeof(struct BNodeTable)/sizeof(int), MPI_INT, info->bnode, 0, sizeof(struct BNodeTable)/sizeof(int), MPI_INT, info->bwin);
    } else {
      int one = 1;
      MPI_Accumulate(&one, 1, MPI_INT, info->bnode, 2, 1, MPI_INT, MPI_SUM, info->bwin);
    }
    MPI_Win_unlock(info->bnode, info->bwin);
    int temp = info->rankprocs[bt.a];
    info->rankprocs[bt.a] = info->rankprocs[bt.b];
    info->rankprocs[bt.b] = temp;
    // SUITCASE EXCHANGE
  }
  
  
  
  printf("rank %d has left the airlock\n", info->rank);
}









