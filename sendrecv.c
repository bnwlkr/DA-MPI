#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "sendrecv.h"
#include "profile.h"
#include "dampi.h"

/*  Now, everything that enters the airlock will have a request that it is waiting on.
 *  So, when a swap occurs, the swapping processes can trade request objects, and the line
 *  number of the send or recv that they were executing when they entered the airlock.
 *  The newly switched process can then enter the send and start waiting on the request 
 *  that it received. 
 *  
 *  This also means that there is no need for a mesage forwarding system. If a process enters the
 *  airlock and finds out that its request concerns the swapping ranks, it can cancel it and resend.
 *
 */

// line: the line number of this DAMPI_Send call
// test: whether this process needs to go straight to testing

int DAMPI_Send(int line, const void *buf, int count, MPI_Datatype datatype, short dest, short tag, MPI_Comm comm) {
  if (info->proc != dest) {
    int inc = 1; 
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->freqwin);
    MPI_Accumulate(&inc, 1, MPI_INT, info->bnode, eoffset(dest, info->rank), 1, MPI_INT, MPI_SUM, info->freqwin);
    MPI_Win_unlock(info->bnode, info->freqwin);
  }
  int dampi_tag = ((dest << 16) | (tag & 0xffff));
  int res = MPI_Isend(buf, count, datatype, info->rankprocs[dest], dampi_tag, comm, &info->request);
  if (res != MPI_SUCCESS) return res;
  test: ;
  MPI_Status status;
  int done = 0;
  while (!done) {
    DAMPI_Airlock();
    MPI_Test(&info->request, &done, &status);
    sleep(2);
  }
  return MPI_SUCCESS;
}


int DAMPI_Recv(void *buf, int count, MPI_Datatype datatype, short source, short tag, MPI_Comm comm, MPI_Status * status) {
  



  return 0;
}
