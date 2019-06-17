#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"


void run () {

// ALLOCATE STUFF TO SUITCASE

while (1) {
  



  DAMPI_Airlock(); // a place where processes go to be migrated (blocks if the first process to enter discovers that a migration is to be performed). check for migrations sometimes, if required busy wait until everyone has joined. if migrate ->> update translation table, could use MPI_Ssend to circumvent the issue of outstanding messages during migration (unless I can think of an elegant solution).
}




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
  
    DAMPI_Profile(proc, n);
    
    DAMPI_Info_sync();
    DAMPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    
    if (proc == 3) {
      DAMPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
    DAMPI_Info_sync();
    
  
    DAMPI_Diag(); 
    
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
