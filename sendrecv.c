#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sendrecv.h"
#include "profile.h"
#include "dampi.h"
#include "migrate.h"

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

static void acc_msg (int dest) {
  if (info->proc != dest) {
    int inc = 1; 
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, info->bnode, 0, info->freqwin);
    MPI_Accumulate(&inc, 1, MPI_INT, info->bnode, eoffset(dest, info->rank), 1, MPI_INT, MPI_SUM, info->freqwin);
    MPI_Win_unlock(info->bnode, info->freqwin);
  }
}

int DAMPI_Send(int line, const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
  acc_msg(dest);
  info->line = line;
  MPI_Request send_request;
  MPI_Request migration_request;
  MPI_Status send_status;
  MPI_Status migration_status;
  int res = MPI_Isend(buf, count, datatype, info->rankprocs[dest], tag, comm, &send_request);
  if (res != MPI_SUCCESS) return res;
  res = MPI_Irecv(NULL, 0, MPI_BYTE, info->rankprocs[dest], MIGREQUEST, MPI_COMM_WORLD, &migration_request);
  if (res != MPI_SUCCESS) return res;
  int send_done = 0;
  int migration_needed = 0;
  while (1) {
    MPI_Test(&send_request, &send_done, &send_status);
    if (send_done) {
      MPI_Cancel(&migration_request);
      MPI_Status cancel_status;
      MPI_Wait(&migration_request, &cancel_status);
      info->line = 0;
      return 0;
    }
    MPI_Test(&migration_request, &migration_needed, &migration_status);
    if (migration_needed) {
      MPI_Cancel(&send_request);
      MPI_Status cancel_status;
      MPI_Wait(&send_request, &cancel_status);
      int cancelled = 0;
      MPI_Test_cancelled(&cancel_status, &cancelled);
      if (cancelled) { return 1; printf("SEND CANCELLED\n"); }
      info->line = 0;
      return 0;
    }
  }
}


int DAMPI_Recv(int line, void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status * status) {
  info->line = line;
  MPI_Request recv_request;
  MPI_Request migration_request;
  MPI_Status recv_status;
  MPI_Status migration_status;
  int res = MPI_Irecv(buf, count, datatype, source == MPI_ANY_SOURCE ? MPI_ANY_SOURCE : info->rankprocs[source], tag, comm, &recv_request);
  if (res != MPI_SUCCESS) return res;
  res = MPI_Irecv(NULL, 0, MPI_BYTE, source == MPI_ANY_SOURCE ? MPI_ANY_SOURCE : info->rankprocs[source], MIGREQUEST, MPI_COMM_WORLD, &migration_request);
  if (res != MPI_SUCCESS) return res;
  int recv_done = 0;
  int migration_needed = 0;
  while (1) {
    MPI_Test(&recv_request, &recv_done, &recv_status);
    if (recv_done) {
      MPI_Cancel(&migration_request);
      MPI_Status cancel_status;
      MPI_Wait(&migration_request, &cancel_status);
      info->line = 0;
      return 0;
    }
    MPI_Test(&migration_request, &migration_needed, &migration_status);
    if (migration_needed) {
      MPI_Cancel(&recv_request);
      MPI_Status cancel_status;
      MPI_Wait(&recv_request, &cancel_status);
      int cancelled = 0;
      MPI_Test_cancelled(&cancel_status, &cancelled);
      if (cancelled) { return 1; printf("RECEIVE_CANCELLED\n"); }
      info->line = 0;
      return 0;
    }
  }
}
