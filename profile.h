#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define TRIALS 100
#define DATA_SIZE 40

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern struct ProcInfo info;

enum MType {
    REQUEST,
    RESPONSE,
    NEXT,
};

// will have to modify this to include a translation service

struct ProcInfo {
 int proc;
 int n;
 int n_edges;
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
void DAMPI_Profile (int rank, int n);

/*  free memory and windows
 *
 */
void DAMPI_Finalize ();

/*  Synchronize processes on bnode's info window
 *
 */

void DAMPI_Info_sync();

/*  get the offset in the edge tables for this edge
 *
 */ 
int DAMPI_Eoffset (int a, int b);

/*  display all DAMPI-relevant information for aiding in diagnosing issues
 *
 */
void DAMPI_Diag();
