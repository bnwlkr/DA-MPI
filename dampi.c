#include "dampi.h"
#include "profile.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


int n_funcs = 0;
dampi_func* funcs;

void DAMPI_Reg(int nf, ...) {
  n_funcs = nf;
  funcs = malloc(sizeof(dampi_func)*nf);
  va_list args;
  va_start(args, nf);
  for (int i = 0; i < nf; i++) {
    funcs[i] = va_arg(args, dampi_func);
  }
  va_end(args);
}


void DAMPI_Start(int proc, int n, dampi_func f, int sc_size, void* suitcase) {
  profile(proc, n, f, sc_size);
  int myfunc;
  for (int i = 0; i < n_funcs; i++) {
    if (funcs[i] == f) {
      myfunc = i;
    }
  }
  int rank_nums[proc == info->bnode ? n*2 : 0];
  MPI_Win win;
  MPI_Win_create(rank_nums, proc==info->bnode ? 2*n*sizeof(int) : 0, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
  MPI_Win_fence(0, win);
  MPI_Put(&myfunc, 1, MPI_INT, info->bnode, info->rank, 1, MPI_INT, win);
  MPI_Put(&sc_size, 1, MPI_INT, info->bnode, info->rank+n, 1, MPI_INT, win);
  MPI_Win_fence(0, win);
  MPI_Win_fence(MPI_MODE_NOPUT, win);
  MPI_Get(&rank_nums, n, MPI_INT, info->bnode, 0, n, MPI_INT, win);
  MPI_Get(info->rank_sc_sizes, n, MPI_INT, info->bnode, n, n, MPI_INT, win);
  MPI_Win_fence(0, win);
  for (int i = 0; i < n; i++) {
    info->rankfuncs[i] = funcs[rank_nums[i]];
  }
  f(suitcase);
}

