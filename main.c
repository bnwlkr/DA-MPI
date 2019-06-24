#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"
#include <time.h>
#include <stdlib.h>


int proc_;
int n_;

void bar (void* arg) {
  while (1) {
    sleep(1);
    DAMPI_Send(NULL, 1, MPI_INT, rand()%n_, 0, MPI_COMM_WORLD);
    DAMPI_Airlock();
  }
  
}

void foo (void* arg) {
  while (1) {
    sleep(1);
    DAMPI_Send(NULL, 1, MPI_INT, rand()%n_, 0, MPI_COMM_WORLD);
    DAMPI_Airlock();
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
    proc_=proc;
    n_=n;
    //printf("proc %d, %s, reporting for duty\n", proc, procname);
    
    srand(time(NULL));
    
    
    struct FooCase {
      int a, b;
    };
    
    struct BarCase {
      int a,b,c;
    };
    
  
    
    DAMPI_Register(proc, n, 2, foo, bar);
    if (!proc) {
      struct FooCase *fc = malloc(sizeof(struct FooCase));
      fc->a = 3;
      fc->b = 4;
      DAMPI_Start(foo, sizeof(struct FooCase), fc);
      free(fc);
    } else {
      struct BarCase *bc = malloc(sizeof(struct BarCase));
      bc->a = bc->b = bc->c = 10;
      DAMPI_Start(bar, sizeof(struct BarCase), bc);
    }

    
    
    MPI_Barrier(MPI_COMM_WORLD);
  
  
    DAMPI_Diag();
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
