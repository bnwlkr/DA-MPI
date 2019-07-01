#include <stdio.h>
#include "dampi.h"

int main (int argc, char* argv[]) {
  int n;
  int proc;
  int len;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc);
  
  DAMPI_Profile(proc, n);
  
  MPI_Finalize();
}
