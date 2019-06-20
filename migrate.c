#include "migrate.h"
//
//#define DAMPI_LOOP DAMPI_Save(); while (DAMPI_Airlock())


void get_bt (struct BNodeTable * bt) {
  MPI_Win_lock(MPI_LOCK_SHARED, info->bnode, 0, info->bwin);
  MPI_Get(bt, sizeof(struct BNodeTable)/sizeof(int) + info->n_edges, MPI_INT, info->bnode, 0, sizeof(struct BNodeTable)/sizeof(int) + info->n_edges*2, MPI_INT, info->bwin);
  MPI_Win_unlock(info->bnode, info->bwin);
}

double value (struct BNodeTable * bt) {
  double sum = 0.0;
  for (int i = 0; i < info->n-1; i++) {
    for (int j = i+1; j < info->n; j++) {
      int offset = eoffset(i,j);
      int freq = bt->freq[offset];
      int iproc = info->rankprocs[i];
      int jproc = info->rankprocs[j];
      double delay = info->delays[eoffset(iproc,jproc)];
      sum += (double)freq/delay;
    }
  }
  return sum;
}










