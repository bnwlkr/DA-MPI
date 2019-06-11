#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "src/profiler.h"

void run (int rank, int n) {
    MPIX_Profile(rank, n);
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
