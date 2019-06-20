#ifndef DAMPI_H
#define DAMPI_H

#include <mpi.h>

void DAMPI_Init(int proc, int n);

int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

void DAMPI_Diag();

void DAMPI_Finalize();


#endif 
