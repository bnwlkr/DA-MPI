#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRIALS 100
#define DATA_SIZE 10


enum MType {
  RESPOND,
  RESPONSE,
  RESULTS,
  GO,
  FINISH
};


/* participate in system profiling.
 * all results should be sent to rank 0;
 */
void MPIX_Profile (int rank, int n);


/*  routine of the data collecting process (rank 0)
 *
 */
static void origin (char* data, int n);

/*  routine of the nodes
 *
 */
static void node (char* data, int rank, int n);


/*  ping a process TRIALS times
 *  
 */
static void ping (char* data, int to);



