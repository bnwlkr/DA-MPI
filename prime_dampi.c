#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"
#include <time.h>
#include <stdlib.h>


typedef enum {FILTERED, PRIME, GOTOAIRLOCK, NEXT, DIE} MESSAGE;

struct GeneratorSC {
  int n;
  int next;
  int primes[];
};


struct WorkerSC {
  int n;
  int rank;
  int prime;
};

void generator(void* arg) {
  struct GeneratorSC * gensc = (struct GeneratorSC*)arg;
  int n = gensc->n;
  MPI_Status status;
  int recv;
  do {
    DAMPI_Send(&gensc->next, 1, MPI_INT, 1, NEXT, MPI_COMM_WORLD);
    gensc->next+=2;
    DAMPI_Recv(&recv, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG==PRIME) {
      gensc->primes[status.MPI_SOURCE] = recv;
      if (status.MPI_SOURCE == n-1) {
        DAMPI_Airlock(0);
        DAMPI_Send(&recv, 1, MPI_INT, 1, DIE, MPI_COMM_WORLD);
        break;
      }
    }
  } while (DAMPI_Airlock(1));
}


void worker (void* arg) {
  struct WorkerSC * worksc = (struct WorkerSC*)arg;
  int n = worksc->n;
  int rank = worksc->rank;
  MPI_Status status;
  int recv = 9;
  do {
    DAMPI_Recv(&recv, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == NEXT) {
      if (worksc->prime == -1 || worksc->prime%recv == 0) {
          DAMPI_Send(&recv, 1, MPI_INT, 0, worksc->prime == -1 ? PRIME : FILTERED, MPI_COMM_WORLD);
          worksc->prime = worksc->prime == -1 ? recv : worksc->prime;
          if (rank < n-1) {
            DAMPI_Send(&worksc->prime, 1, MPI_INT, rank+1, GOTOAIRLOCK, MPI_COMM_WORLD);
          }
        } else if (worksc->prime%recv) {
          DAMPI_Send(&recv, 1, MPI_INT, rank+1, NEXT, MPI_COMM_WORLD);
        }
    } else if (status.MPI_TAG == GOTOAIRLOCK) {
      if (rank < n-1) { 
        DAMPI_Send(&worksc->prime, 1, MPI_INT, rank+1, GOTOAIRLOCK, MPI_COMM_WORLD);
      }
    } else if (status.MPI_TAG == DIE) {
      if (rank < n-1) {
        DAMPI_Send(&worksc->prime, 1, MPI_INT, rank+1, DIE, MPI_COMM_WORLD);
      }
      break;
    }
  } while (DAMPI_Airlock(1));
}



int main(int argc, char** argv) {
    int n;
    int rank;
    char procname[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(procname, &len);
  
    
    if (DAMPI_Register(rank, n, 2, generator, worker)) {
      printf("no profile file found for this configuration\n");
      MPI_Finalize();
      return 0;
    }
    
    struct GeneratorSC* gensc;
    struct WorkerSC* worksc;
    
    switch (rank) {
      case 0: {
        gensc = malloc(sizeof(struct GeneratorSC) + n*sizeof(int));
        gensc->n = n;
        gensc->next = 3;
        gensc->primes[0] = 2;
        DAMPI_Start(generator, sizeof(struct GeneratorSC) + n*sizeof(int), (void**)&gensc);
        break;
      }
      default: {
        worksc = malloc(sizeof(struct WorkerSC));
        worksc->n = n;
        worksc->rank = rank;
        worksc->prime = -1;
        DAMPI_Start(worker, sizeof(struct WorkerSC), (void**)&worksc);
        break;
      }
    }
    
    if (!DAMPI_Rank()) {
      struct GeneratorSC* ogensc = ((struct GeneratorSC*)(rank==0?(void*)gensc:(void*)worksc));
      printf("[");
      for (int i = 0; i < n; i++) {
        printf(" %d ", ogensc->primes[i]);
      }
      printf("]\n");
    }
    
    free(gensc);
    free(worksc);
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
