#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <time.h>

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef enum {FILTERED, PRIME} RESPONSE;

int * latencies;
int proc_;
int n_;

int _boffset (int a) {
  return (n_-1)*a - (a-1)*a/2;
}

int _eoffset (int a, int b) {
  int min = MIN(a,b);
  int max = MAX(a,b);
  int boffset_ = _boffset(min);
  return boffset_+max-min-1;
}


int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = latencies[_eoffset(proc_, dest)];
  nanosleep (&ts, NULL);
  return PMPI_Ssend(buf, count, datatype, dest, tag, comm);
}

void generator (int n) {
  MPI_Status status;
  int * primes = malloc(n*sizeof(int));
  primes[0] = 2;
  int next = 3;
  while(1) { 
    MPI_Send(NULL, 0, MPI_INT, 1, next, MPI_COMM_WORLD); // send next thing into the pipeline
    next+=2;
    RESPONSE r;
    MPI_Recv(&r, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
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
  MPI_Send(&r, 1, MPI_INT, 0, prime, MPI_COMM_WORLD); // send prime to generator
  int value;
  while (1) {
    MPI_Recv(NULL, 0, MPI_INT, rank-1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    value = status.MPI_TAG;
    if (!value && rank < n-1) {
      MPI_Send(NULL, 0, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
      break;
    } else if (!value && rank == n-1) {
      break;
    }
    if (prime%value) {
      MPI_Send(NULL, 0, MPI_INT, rank+1, value, MPI_COMM_WORLD);
    } else {
      r = FILTERED;
      MPI_Send(&r, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
  }
}


 
int main(int argc, char* argv[]) {
    int n;
    int rank;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    proc_ = rank;
    n_=n;
    
    int n_edges = n*(n-1)/2; 
    
    latencies = malloc(n_edges*sizeof(int));
    char filename[11];
    sprintf(filename, "lat_%d", n);
    FILE * f = fopen(filename, "r");
    if (!f) {
      printf ("missing latency file for this configuratinon\n");
      MPI_Finalize();
      return 1;
    }
    char buf[15];
    for (int i = 0; i < n_edges; i++) {
      fgets(buf, 10, f);
      latencies[i] = atoi(buf);
    }
    
    
    
    
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

