#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dampi.h"
#include "profile.h"

dampi_func* funcs;
int n_funcs;

void DAMPI_Register(int proc, int n, int nf, ...) {
  profile(proc, n);
  funcs = malloc(sizeof(dampi_func)*nf);
  n_funcs = nf;
  va_list args;
  va_start(args, nf);
  for (int i = 0; i < nf; i++) {
    funcs[i] = va_arg(args, dampi_func);
  }
  va_end(args);
}

int DAMPI_Rank() {
  return info->rank;
}

void DAMPI_Start(dampi_func f, int sc_size, void** suitcase) {
  int n = info->n;
  int myfunc;
  for (int i = 0; i < n_funcs; i++) {
    if (funcs[i] == f) {
      myfunc = i;
    }
  }
  int rank_nums[n*2];
  MPI_Win win;
  MPI_Win_create(rank_nums, info->proc==info->bnode ? 2*n*sizeof(int) : 0, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
  MPI_Win_fence(0, win);
  MPI_Put(&myfunc, 1, MPI_INT, info->bnode, info->rank, 1, MPI_INT, win);
  MPI_Put(&sc_size, 1, MPI_INT, info->bnode, info->rank+n, 1, MPI_INT, win);
  MPI_Win_fence(0, win);
  MPI_Bcast(rank_nums, n*2, MPI_INT, info->bnode, MPI_COMM_WORLD);
  int max_size = 0;
  for (int i = 0; i < n; i++) {
    if (rank_nums[n+i] > max_size) max_size = rank_nums[n+i];
    info->rankfuncs[i] = funcs[rank_nums[i]];
  }
  info->sc_size = max_size;
  info->suitcase = *suitcase = realloc(*suitcase, info->sc_size);
  MPI_Win_create(info->suitcase, info->sc_size, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &info->scwin);
  f(info->suitcase);
}

