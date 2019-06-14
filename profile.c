#include "profile.h"


void MPIX_Profile (int rank, int n, double * results) {
  char* data  = malloc(DATA_SIZE);
  MPI_Barrier(MPI_COMM_WORLD);
  switch (rank) {
    case 0:
      origin(data, n, results);
      break;
    default:
      node(data, rank, n);
  }
}

static void measure (char* data, int rank, int n, double * results) {
  for (int i = 0; i < n; i++) {
    if (i != rank) {
      MPI_Wtime();
      for (int j = 0; j < TRIALS; j++) {
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, i, REQUEST, MPI_COMM_WORLD);
        MPI_Recv(data, DATA_SIZE, MPI_BYTE, i, RESPONSE, MPI_COMM_WORLD, NULL);
      }
      results[i] = MPI_Wtime();
    }
  }
}

static void origin (char* data, int n, double * results) {
  MPI_Status status;
  measure(data, 0, n, results);
  MPI_Ssend(data, DATA_SIZE, MPI_BYTE, 1, GO, MPI_COMM_WORLD);
  double times[n];
  int done = 0;
  while (!done) {
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch (status.MPI_TAG) {
      case REQUEST:
        MPI_Recv(data, DATA_SIZE, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, status.MPI_SOURCE, RESPONSE, MPI_COMM_WORLD);
        break;
      case RESULTS:
        MPI_Recv(&results[status.MPI_SOURCE*n], n, MPI_DOUBLE, MPI_ANY_SOURCE, RESULTS, MPI_COMM_WORLD, &status);
        if (status.MPI_SOURCE == n-1) {
          MPI_Ssend(data, DATA_SIZE, MPI_BYTE, 1, FINISH, MPI_COMM_WORLD);
          done = 1;
        }
    }
  }
}

static void node (char* data, int rank, int n) {
  MPI_Status status;
  double * times = calloc(sizeof(double), n);
  int done = 0;
  while (!done) {
    MPI_Recv(data, DATA_SIZE, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    switch(status.MPI_TAG) {
      case REQUEST:
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, status.MPI_SOURCE, RESPONSE, MPI_COMM_WORLD);
        break;
      case GO:
        measure(data, rank, n, times);
        MPI_Ssend(times, n, MPI_DOUBLE, 0, RESULTS, MPI_COMM_WORLD);
        if (rank < n-1) {
          MPI_Ssend(data, DATA_SIZE, MPI_BYTE, rank+1, GO, MPI_COMM_WORLD);
        }
        break;
      case FINISH:
        if (rank < n-1) {
          MPI_Ssend(data, DATA_SIZE, MPI_BYTE, rank+1, FINISH, MPI_COMM_WORLD);
        }
        done = 1;
    }
  }
}
  
