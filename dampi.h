#ifndef DAMPI_H
#define DAMPI_H

#include <mpi.h>


typedef void(*dampi_func)(void*);

/*  Register which functions DAMPI will migrate between
 *  nf: number of functions to register
 *  proc: process number (original rank)
 *  n: number of processes in the COMM
 */ 
int DAMPI_Register(int proc, int n, int nf, ...);

/*  Profile MPI system and start this rank running f
 *  f: function that this rank will be running
 *  sc_size: size of data argument to this rank's function
 *  suitcase: this rank's data argument
 */
void DAMPI_Start(dampi_func f, int sc_size, void** suitcase);

/*  DAMPI's Send function.
 */
int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

int DAMPI_Rank();

void DAMPI_Profile(int proc, int n);

void DAMPI_Finalize();

int DAMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status * status);

/*  View the state of the system
 */
void DAMPI_Diag();

/*  The place where all processes go regularly to particpate in migrations. 
 */
int DAMPI_Airlock(int migrate);

/* Cleanup function. Free windows and data
*/
void DAMPI_Finalize();


#endif 
