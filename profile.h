#ifndef PROFILE_H
#define PROFILE_H

#include <mpi.h>

#define TRIALS 100
#define DATA_SIZE 40

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern struct ProcInfo * info;

typedef void(*f)(void* x);

enum MType {
    REQUEST,
    RESPONSE,
    NEXT,
};

struct BNodeTable {
  int a, b;             // nodes that currently need to be migrated (-1 if none)
  int freq[];           // comms frequencies
};

struct ProcInfo {
 int proc, rank, n, n_edges, bnode;
 MPI_Win bwin;
 double* delays;
 struct BNodeTable* bt;
 f* rankfuncs;                // rank->function map
 int* rank_sc_sizes;
 int* rankprocs;              // rank->process map
};

/* called by all processes to participate in system profiling.
 * allocate windows on all processes that will share profiling data amongst eachother
 * select the 'best-connected' node and allocate a frequency table on it, notify every node of the best connected node
 * 
 */
void profile (int rank, int n);



/*  get the offset in the edge tables for this edge
 *
 */ 
int eoffset (int a, int b);


/*  get the offset in the edge tables for this edge
 *
 */ 
int boffset (int a);


#endif
