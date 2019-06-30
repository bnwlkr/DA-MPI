#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
 


typedef enum {FILTERED, PRIME} RESPONSE;


void generator (int n) {
  MPI_Status status;
  int * primes = malloc(n*sizeof(int));
  primes[0] = 2;
  int next = 3;
  while(1) {
    MPI_Ssend(NULL, 0, MPI_INT, 1, next, MPI_COMM_WORLD); // send next thing into the pipeline
    next+=2;
    RESPONSE r;
    MPI_Recv(&r, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // pipeline completed processing last input
    if (r == PRIME) {
      primes[status.MPI_SOURCE] = status.MPI_TAG; // add new prime to list
      if (status.MPI_SOURCE == n-1) {
        break;
      }
    }
  }
  printf("[");
  for (int i = 0; i < n; i++) {
    printf(" %d ", primes[i]);
  }
  printf("]\n");
  free(primes);
  MPI_Ssend(NULL, 0, MPI_INT, 1, 0, MPI_COMM_WORLD);
}


void worker (int rank, int n) {
  MPI_Status status;
  MPI_Recv(NULL, 0, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
  int prime = status.MPI_TAG;
  RESPONSE r = PRIME;
  MPI_Ssend(&r, 1, MPI_INT, 0, prime, MPI_COMM_WORLD); // send prime to generator
  int value;
  while (1) {
    MPI_Recv(NULL, 0, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    value = status.MPI_TAG;
    if (!value && rank < n-1) {
      MPI_Ssend(NULL, 0, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
      break;
    } else if (!value && rank == n-1) {
      break;
    }
    if (prime%value) {
      MPI_Ssend(NULL, 0, MPI_INT, rank+1, value, MPI_COMM_WORLD);
    } else {
      r = FILTERED;
      MPI_Ssend(&r, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
  }
}


 
int main(int argc, char* argv[]) {
    int n;
    int rank;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    switch (rank) {
      case 0:
        generator(n);
        break;
      default: 
        worker(rank, n);
        break;
        
    }
 
    MPI_Finalize();
    
    return 0;
}

