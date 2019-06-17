#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
