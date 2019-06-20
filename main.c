#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"


int main(int argc, char** argv) {
    int n;
    int proc;
    char procname[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc);
    MPI_Get_processor_name(procname, &len);
    
    printf("proc %d, %s, reporting for duty\n", proc, procname);
    
    
    DAMPI_Init(proc, n);
    
    DAMPI_Send(NULL, 0, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    DAMPI_Send(NULL, 0, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    
  
    

    
    MPI_Barrier(MPI_COMM_WORLD);
  
  
    DAMPI_Diag();
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
