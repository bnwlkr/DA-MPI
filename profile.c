#include "profile.h"


void MPIX_Profile (int rank, int n) {
  int n_edges = n*(n-1)/2;
  double * delays = calloc(n_edges, sizeof(double));
  MPI_Win win;
  MPI_Win_create(delays, n_edges*sizeof(double), sizeof(double), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
  char data[DATA_SIZE];
  
  switch (rank) {
    case 0:
      measure(rank, n, data, delays);
      MPI_Ssend(data, DATA_SIZE, MPI_BYTE, 1, NEXT, MPI_COMM_WORLD);
      break;
    default:
      respond(data);
      measure(rank, n, data, &delays[(n-1)*rank-(rank-1)*rank/2]);
      if (rank < n-1) {
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, rank+1, NEXT, MPI_COMM_WORLD);
      }
  }
  
  MPI_Win_fence(0, win);
  for (int i = 0; i < n; i++) {
    if (i != rank) {
     int n_read = n-i-1;
     int offset = (n-1)*i - (i-1)*i/2;
     MPI_Get(&delays[offset], n_read, MPI_DOUBLE, i, offset, n_read, MPI_DOUBLE, win);
    }
  }
  MPI_Win_fence(0, win);
  
  if (!rank) {
   for (int i = 0; i < n_edges; i++) {
      printf("delays[%d] = %f\n", i, delays[i]);  
   } 
  }
  

  MPI_Win_free(&win);
  free(delays);
}


static void respond (char* data) {
  MPI_Status status;
  int done = 0;
  while (!done) {
    MPI_Recv(data, DATA_SIZE, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch (status.MPI_TAG) {
      case REQUEST:
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, status.MPI_SOURCE, RESPONSE, MPI_COMM_WORLD);
        break;
      default:
        return;
    }
  }
}

static void measure (int rank, int n, char* data, double* results) {
  for (int i = rank+1; i < n; i++) {
    double t0 = MPI_Wtime();
    for (int j = 0; j < TRIALS; j++) {
      MPI_Ssend(data, DATA_SIZE, MPI_BYTE, i, REQUEST, MPI_COMM_WORLD);
      MPI_Recv(data, DATA_SIZE, MPI_BYTE, i, RESPONSE, MPI_COMM_WORLD, NULL);
    }
    double t1 = MPI_Wtime();
    results[i-(rank+1)] = t1-t0;
  }
}


