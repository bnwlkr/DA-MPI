#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "dampi.h"


int proc_;
int n_;


void zero (void* arg) {
  switch (DAMPI_Line()) { case -1:
    while (1) {
      sleep(rand()%5);
      int send;
      case __LINE__: DAMPI_Send(__LINE__, &send, 1, MPI_INT, rand()%n_, 0, MPI_COMM_WORLD);
    }
  }
}

void one (void* arg) {
  switch (DAMPI_Line()) { case -1:
    while (1) {
      sleep(rand()%5);
      int send;
      case __LINE__: DAMPI_Send(__LINE__, &send, 1, MPI_INT, rand()%n_, 0, MPI_COMM_WORLD);
    }
  }
}

void two (void* arg) {
  switch (DAMPI_Line()) { case -1:
    while (1) {
      sleep(rand()%5);
      int send;
      case __LINE__: DAMPI_Send(__LINE__, &send, 1, MPI_INT, rand()%n_, 0, MPI_COMM_WORLD);
    }
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
    printf("proc %d, %s, reporting for duty\n", proc, procname);
    
    srand(time(NULL));
    
    struct ZeroCase {
      int a,b;
    };
    
    struct TwoCase {
      int a,b,c;
    };
    
    struct OneCase {
      int a,b,c,d;
    };
  
    
    DAMPI_Register(proc, n, 3, zero, one, two);
    
    int send;
    
    DAMPI_Send(__LINE__, &send, 1, MPI_INT, rand()%n_, 0, MPI_COMM_WORLD);
    return 0;
    
    
    switch(proc) {
      case 0:
        {
        struct ZeroCase * zerosc = malloc(sizeof(struct ZeroCase));
        DAMPI_Start(zero, sizeof(struct ZeroCase), zerosc);
        break;
        }
      case 1:
        {
        struct OneCase * onesc = malloc(sizeof(struct OneCase));
        DAMPI_Start(one, sizeof(struct OneCase), onesc);
        break;
        }
      case 2:
        {
        struct TwoCase * twosc = malloc(sizeof(struct TwoCase));
        DAMPI_Start(two, sizeof(struct TwoCase), twosc);
        break;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
  
    DAMPI_Diag();
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
