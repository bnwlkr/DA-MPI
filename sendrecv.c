#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sendrecv.h"
#include "profile.h"
#include "dampi.h"

#define HEADER_SIZE 

int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, short dest, short tag, MPI_Comm comm) {
  if (info->proc != dest) {
    int inc = 1; 
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->freqwin);
    MPI_Accumulate(&inc, 1, MPI_INT, info->bnode, eoffset(dest, info->rank), 1, MPI_INT, MPI_SUM, info->freqwin);
    MPI_Win_unlock(info->bnode, info->freqwin);
  }
  int dampi_tag = ((dest << 16) | (tag & 0xffff));
  MPI_Request request;
  int res = MPI_Isend(buf, count, datatype, info->rankprocs[dest], dampi_tag, comm, &request);
  if (res != MPI_SUCCESS) return res;
  int done = 0;
  while (!done) {
    DAMPI_Airlock();
    MPI_Test(&request, &done, NULL);
  }
  return res;
}


int DAMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status * status) {
  



  return 0;
}
