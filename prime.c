#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"
#include <time.h>
#include <stdlib.h>
#include "profile.h"

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
//    printf("GENERATOR WAIT SEND\n");
    DAMPI_Send(&gensc->next, 1, MPI_INT, 1, NEXT, MPI_COMM_WORLD);
    gensc->next+=2;
//    printf("GENERATOR WAIT RECV\n"); 
    DAMPI_Recv(&recv, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    printf("GENERATOR RECV %d FROM %d\n", recv, status.MPI_SOURCE);
    if (status.MPI_TAG==PRIME) {
      gensc->primes[status.MPI_SOURCE] = recv;
      if (status.MPI_SOURCE == n-1) {
        DAMPI_Airlock(0);
        break;
      }
    }
  } while (DAMPI_Airlock(1));
  printf("[");
  for (int i = 0; i < n; i++) {
    printf(" %d ", gensc->primes[i]);
  }
  printf("]\n");
  DAMPI_Send(&recv, 1, MPI_INT, 1, DIE, MPI_COMM_WORLD);
}


void worker (void* arg) {
  struct WorkerSC * worksc = (struct WorkerSC*)arg;
  int n = worksc->n;
  int rank = worksc->rank;
  MPI_Status status;
  int recv = 9;
  do {
//    printf("%d WAIT RECV\n", worksc->rank);
    DAMPI_Recv(&recv, 1, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    if (status.MPI_TAG == NEXT) {
      printf("%d (with prime %d) RECV %d\n", worksc->rank, worksc->prime, recv);
      if (worksc->prime == -1 || worksc->prime%recv == 0) {
//          printf("%d WAIT SEND TO GENERATOR\n", worksc->rank);
          DAMPI_Send(&recv, 1, MPI_INT, 0, worksc->prime == -1 ? PRIME : FILTERED, MPI_COMM_WORLD);
          printf("%d SEND %d TO GENERATOR\n", worksc->rank, recv);
          worksc->prime = worksc->prime == -1 ? recv : worksc->prime;
          if (rank < n-1) {
//            printf("%d WAIT SEND GOTOAIRLOCK\n", worksc->rank);
            DAMPI_Send(&worksc->prime, 1, MPI_INT, rank+1, GOTOAIRLOCK, MPI_COMM_WORLD);
            printf("%d SEND GOTOAIRLOCK\n", worksc->rank);
          }
        } else if (worksc->prime%recv) {
//          printf("%d WAIT SEND NEXT\n", worksc->rank);
          DAMPI_Send(&recv, 1, MPI_INT, rank+1, NEXT, MPI_COMM_WORLD);
          printf("%d SEND NEXT\n", worksc->rank);
        }
    } else if (status.MPI_TAG == GOTOAIRLOCK) {
      if (rank < n-1) {
//          printf("%d WAIT FORWARD GOTOAIRLOCK\n", worksc->rank); 
          DAMPI_Send(&worksc->prime, 1, MPI_INT, rank+1, GOTOAIRLOCK, MPI_COMM_WORLD);
          printf("%d FORWARD GOTOAIRLOCK\n", worksc->rank);
        }
    } else if (status.MPI_TAG == DIE) {
      if (rank < n-1) {
//          printf("%d WAIT FORWARD DIE\n", worksc->rank);
          DAMPI_Send(&worksc->prime, 1, MPI_INT, rank+1, DIE, MPI_COMM_WORLD);
          printf("%d FORWARD DIE\n", worksc->rank);
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
  
    
    DAMPI_Register(rank, n, 2, generator, worker);
    
    switch (rank) {
      case 0: {
        struct GeneratorSC * gensc = malloc(sizeof(struct GeneratorSC) + n*sizeof(int));
        gensc->n = n;
        gensc->next = 3;
        gensc->primes[0] = 2;
        DAMPI_Start(generator, sizeof(struct GeneratorSC) + n*sizeof(int), gensc);
        free(gensc);
        break;
      }
      default: {
        struct WorkerSC * worksc = malloc(sizeof(struct WorkerSC));
        worksc->n = n;
        worksc->rank = rank;
        worksc->prime = -1;
        DAMPI_Start(worker, sizeof(struct WorkerSC), worksc);
        free(worksc);
        break;
      }
    }
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
