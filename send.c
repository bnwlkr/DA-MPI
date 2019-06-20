#include "send.h"
#include "profile.h"
#include "dampi.h"


int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  if (info->proc != dest) {
    int inc = 1; 
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->bwin);
    MPI_Accumulate(&inc, 1, MPI_INT, info->bnode, eoffset(dest, info->proc) + sizeof(struct BNodeTable)/sizeof(int) , 1, MPI_INT, MPI_SUM, info->bwin);
    MPI_Win_unlock(info->bnode, info->bwin);
  }
  return MPI_Send(buf, count, datatype, dest, tag, comm);
}
