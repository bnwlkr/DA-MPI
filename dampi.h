#ifndef DAMPI_H
#define DAMPI_H

#include <mpi.h>

typedef void(*dampi_func)(void*);

void DAMPI_Start(int proc, int n, dampi_func f, int sc_size);

int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

void DAMPI_Diag();

void DAMPI_Finalize();


#endif 
