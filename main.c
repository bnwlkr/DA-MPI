#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "profile.h"


int main(int argc, char** argv) {
    int n;
    int rank;
    char procname[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(procname, &len);
    
    
    printf("rank %d reporting for duty from %s\n", rank, procname);
    
    
    MPIX_Profile(rank, n);
    
    
    MPI_Finalize();
    return 0;
}
