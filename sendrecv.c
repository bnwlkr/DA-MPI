#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sendrecv.h"
#include "profile.h"
#include "dampi.h"
#include "migrate.h"

int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  if (info->proc != dest) {
    int inc = 1; 
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->freqwin);
    MPI_Accumulate(&inc, 1, MPI_INT, info->bnode, eoffset(dest, info->rank), 1, MPI_INT, MPI_SUM, info->freqwin);
    MPI_Win_unlock(info->bnode, info->freqwin);
  }
  return MPI_Send(buf, count, datatype, info->rankprocs[dest], tag, comm);            
}


int DAMPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status * status) {
  return MPI_Recv(buf, count, datatype, info->rankprocs[source], tag, comm, status);
}
