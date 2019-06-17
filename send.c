#include "profile.h"
#include "send.h"


int DAMPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  if (info.proc != dest) {
    int inc = 1;  
    MPI_Accumulate(&inc, 1, MPI_INT, info.bnode, DAMPI_Eoffset(info.proc, dest), 1, MPI_INT, MPI_SUM, info.win);
  }
  return MPI_Send(buf, count, datatype, dest, tag, comm);
}
