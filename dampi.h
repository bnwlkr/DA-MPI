#ifndef DAMPI_H
#define DAMPI_H

#include <mpi.h>

typedef void(*dampi_func)(void*);

/*  Register which functions DAMPI will migrate between
 *  nf: number of functions to register
 */ 
void DAMPI_Register(int proc, int n, int nf, ...);

/*  Profile MPI system and start this rank running f
 *  sc_size: size of data argument to this rank's function
 */
void DAMPI_Start(dampi_func f, int sc_size, void* suitcase);

int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

void DAMPI_Diag();

int DAMPI_Airlock();

void DAMPI_Finalize();


#endif 
