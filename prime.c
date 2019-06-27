#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"
#include <time.h>
#include <stdlib.h>


void generator(void* arg) {
  





}


void worker (void* arg) {



  
  
  
  
  
  
  
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
    
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
