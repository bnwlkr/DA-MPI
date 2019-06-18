#include "profile.h"
#include "send.h"


int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  if (info.proc != dest) {
    int inc = 1; 
    MPI_Win_lock(MPI_LOCK_SHARED, dest, 0, info.bwin);
    MPI_Accumulate(&inc, 1, MPI_INT, info.bnode, 2, 1, MPI_INT, MPI_SUM, info.bwin);
    MPI_Win_unlock(0, info.bwin);
  }
  return MPI_Send(buf, count, datatype, dest, tag, comm);
}
