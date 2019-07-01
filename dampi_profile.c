#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "dampi.h"
#include "profile.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int* latencies;
int proc_;

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  usleep(latencies[eoffset(proc_, dest)]);
  return PMPI_Send(buf, count, datatype, dest, tag, comm);
}


int main (int argc, char* argv[]) {
  int n;
  int proc;
  int len;
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &n);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc);
  
  proc_=proc;
  srand(time(NULL));
  int n_edges = n*(n-1)/2;
  latencies = malloc(sizeof(int)*n_edges);
  for (int i = 0; i < n_edges; i++) {
    latencies[i] = rand()%1000000;
  }

  
  DAMPI_Profile(proc, n);
  
  
  if (!proc) {
    char filename[11];
    sprintf(filename, "lat_%d", n);
    FILE* f = fopen(filename, "w");
    for (int i = 0; i < info->n_edges; i++) {
      fprintf(f, "%d\n", latencies[i]);
    }
    fclose(f);
  }
  
  MPI_Finalize();
}
