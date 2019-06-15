#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRIALS 100
#define DATA_SIZE 40

enum MType {
    REQUEST,
    RESPONSE,
    NEXT,
};

/* called by all processes to participate in system profiling.
 * allocate windows on all processes that will share profiling data amongst eachother
 * select the 'best-connected' node and allocate a frequency table on it, notify every node of the best connected node
 * 
 */
void MPIX_Profile (int rank, int n);


/*  ping all other processes, and store the timings in results [0,n)
 *  
 */
static void measure (int rank, int n, char* data, double* results);


/*  respond to other processes
 *
 */
static void respond (char* data);


