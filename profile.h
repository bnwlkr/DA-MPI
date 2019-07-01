#ifndef PROFILE_H
#define PROFILE_H

#include <mpi.h>
#include "dampi.h"

#define TRIALS 1
#define DATA_SIZE 1

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern struct ProcInfo * info;


enum MType {
    REQUEST,
    RESPONSE,
    NEXT,
};

struct BNodeTable {
  int a, b, valid;              // nodes that currently need to be migrated (-1 if none), and whether migration is valid
  int * freq;            // comms frequencies
};

struct ProcInfo {
 int proc, rank, n, n_edges, bnode, sc_size;
 MPI_Win bwin, freqwin, scwin;
 double* delays;
 struct BNodeTable* bt;
 int* rankprocs;                  // rank->process map
 dampi_func* rankfuncs;           // rank->func map
 void* suitcase;
};

/* called by all processes to participate in system profiling.
 * allocate windows on all processes that will share profiling data amongst eachother
 * select the 'best-connected' node and allocate a frequency table on it, notify every node of the best connected node
 * 
 */
void profile ();


/*  get the offset in the edge tables for this edge
 *
 */ 
int eoffset (int a, int b);


/*  get the offset in the edge tables for this edge
 *
 */ 
int boffset (int a);


#endif
