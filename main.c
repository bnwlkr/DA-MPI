#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"

void bar (void* arg) {
  DAMPI_Airlock();
}

void foo (void* arg) {
  DAMPI_Airlock();
}

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
    
    DAMPI_Reg(2, foo, bar);
    if (!proc)
      DAMPI_Start(proc, n, foo, 100, NULL);
    else
      DAMPI_Start(proc, n, bar, 30, NULL);
  

    
  

    
    MPI_Barrier(MPI_COMM_WORLD);
  
  
    DAMPI_Diag();
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
