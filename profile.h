#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TRIALS 100
#define DATA_SIZE 40

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


enum MType {
    REQUEST,
    RESPONSE,
    NEXT,
};

// will have to modify this to include a translation service

struct ProcInfo {
 int proc;
 int n;
 double* delays;
 int bnode;
 MPI_Win win;
 int* wbase;
};

/* called by all processes to participate in system profiling.
 * allocate windows on all processes that will share profiling data amongst eachother
 * select the 'best-connected' node and allocate a frequency table on it, notify every node of the best connected node
 * 
 */
void MPIX_Profile (int rank, int n);


/*  get the frequency of communication between rank A and rank B
 *
 */
int frequency (int a, int b); 


