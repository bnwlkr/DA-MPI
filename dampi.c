#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dampi.h"
#include "profile.h"

dampi_func* funcs;
int n_funcs;

static void init_info (int proc, int n) {
  info = malloc(sizeof(struct ProcInfo));
  info->rankprocs = malloc(sizeof(int)*n);
  info->rankfuncs = malloc(sizeof(dampi_func)*n);
  info->proc = proc;
  info->rank = proc;
  info->n = n;
  info->n_edges = n*(n-1)/2;
  info->delays = calloc(info->n_edges, sizeof(double));
}


static void init_windows () {
  MPI_Win_allocate(sizeof(struct BNodeTable), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &info->bt, &info->bwin);
  MPI_Win_allocate(info->n_edges*sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &info->bt->freq, &info->freqwin);
  memset(info->bt->freq, 0, info->n_edges*sizeof(int));
  info->bt->a = info->bt->b = -1;
  info->bt->valid = 1;
  for (int i = 0; i < info->n; i++) {
    info->rankprocs[i] = i;
  }
}

static void destroy_info () {
  free(info->rankprocs);
  free(info->rankfuncs);
  free(info->delays);
  free(info);
}

static void destroy_windows () {
  MPI_Win_free(&info->bwin);
  MPI_Win_free(&info->freqwin);
}

static int read_profile () {
  char filename[11];
  sprintf(filename, "dampi_p_%d", info->n);
  FILE * f = fopen(filename, "r");
  if (!f) return 1;
  char buf[15];
  fgets(buf, 10, f);
  info->bnode = atoi(buf);
  for (int i = 0; i < info->n_edges; i++) {
    fgets(buf, 10, f);
    info->delays[i] = atof(buf);
  }
  return 0;
}

static void write_profile () {
  if (info->proc == 0) {
    char filename[11];
    sprintf(filename, "dampi_p_%d", info->n);
    FILE* f = fopen(filename, "w");
    fprintf(f, "%d\n", info->bnode);
    for (int i = 0; i < info->n_edges; i++) {
      fprintf(f, "%f\n", info->delays[i]);
    }
    fclose(f);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}

void DAMPI_Finalize () { 
  destroy_info ();
  destroy_windows ();
}


int DAMPI_Register(int proc, int n, int nf, ...) {
  init_info(proc, n);
  init_windows();
  if (read_profile()) {
    return 1;
  }
  funcs = malloc(sizeof(dampi_func)*nf);
  n_funcs = nf;
  va_list args;
  va_start(args, nf);
  for (int i = 0; i < nf; i++) {
    funcs[i] = va_arg(args, dampi_func);
  }
  va_end(args);
  return 0;
}

void DAMPI_Profile (int proc, int n) {
  init_info(proc, n);
  init_windows();
  profile();
  write_profile();
  read_profile();
  destroy_info();
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

