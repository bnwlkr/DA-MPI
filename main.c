#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "profiler.h"

void run (int rank, int n) {
   double * results = calloc (sizeof(double), n*n);
   MPIX_Profile(rank, n, results);
   if (!rank) {
     for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          if (i != j) {
            printf("%d --> %d: %f\n", i, j, results[i*n + j]);
          }
        }
     }
  }
}


int main(int argc, char** argv) {
    int n;
    int rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    run (rank, n);
    MPI_Finalize();
    return 0;
}
