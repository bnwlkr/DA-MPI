#include "dampi.h"

void DAMPI_Init(int proc, int n) {
  DAMPI_Profile(proc, n);
  MPI_Win_allocate(sizeof(jmp_buf), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &info.env, &info.envwin);
}
