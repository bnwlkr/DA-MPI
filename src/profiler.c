#include "profiler.h"


static void ping (char* data, int to) {
  for (int i = 0; i < TRIALS; i++) {
   printf("sending data to: %d\n", to);
   MPI_Ssend(data, DATA_SIZE, MPI_BYTE, to, RESPOND, MPI_COMM_WORLD);
   MPI_Recv(data, DATA_SIZE, MPI_BYTE, to, RESPONSE, MPI_COMM_WORLD, NULL);
  }
}

static void origin (char* data, int n) {
  printf("origin starting\n");
  double* results = calloc(sizeof(double), n*n);
  MPI_Status status;
  for (int i = 1; i < n; i++) {
    MPI_Wtime();
    ping(data, i);
    double time = MPI_Wtime();
    results[i] = time;
  }
  MPI_Ssend(data, DATA_SIZE, MPI_BYTE, 1, GO, MPI_COMM_WORLD);
  double times[n];
  while (1) {
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == RESPOND) {
      MPI_Recv(data, DATA_SIZE, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Ssend(data, DATA_SIZE, MPI_BYTE, status.MPI_SOURCE, RESPONSE, MPI_COMM_WORLD);
    } else if (status.MPI_TAG == RESULTS) {
      MPI_Recv(&times, n, MPI_DOUBLE, MPI_ANY_SOURCE, RESULTS, MPI_COMM_WORLD, &status);
      for (int i = 0; i < n; i++) {
        results[status.MPI_SOURCE*n + i] = times[i];
      }
      if (status.MPI_SOURCE == n-1) {
        printf("The results are in!\n");
        for (int i = 0; i < n*n; i++) {
          printf("%f\n", results[i]);
        }
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, 1, FINISH, MPI_COMM_WORLD);
        break;
      }
    }
  }
}


static void node (char* data, int rank, int n) {
  MPI_Status status;
  while (1) {
    MPI_Recv(data, DATA_SIZE, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    if (status.MPI_TAG == RESPOND) {
      MPI_Ssend(data, DATA_SIZE, MPI_BYTE, status.MPI_SOURCE, RESPONSE, MPI_COMM_WORLD);
    } else if (status.MPI_TAG == GO) {
      double * times = calloc(sizeof(double), n);
      for (int i = 0; i < n; i++) {
        if (i != rank) {
         MPI_Wtime();
         ping(data, i);
         times[i] = MPI_Wtime();
        }
      }
      MPI_Ssend(times, n, MPI_DOUBLE, 0, RESULTS, MPI_COMM_WORLD);
      if (rank < n-1) {
       MPI_Ssend(data, DATA_SIZE, MPI_BYTE, rank+1, GO, MPI_COMM_WORLD);
      }
    } else if (status.MPI_TAG == FINISH) {
      if (rank < n-1) {
        MPI_Ssend(data, DATA_SIZE, MPI_BYTE, rank+1, FINISH, MPI_COMM_WORLD);
      }
      break;
    }
  }
}



void MPIX_Profile (int rank, int n) {
  char* data  = malloc(DATA_SIZE);
  MPI_Barrier(MPI_COMM_WORLD);
  switch (rank) {
    case 0:
      origin(data, n);
      break;
    default:
      node(data, rank, n);
  }
}
