#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"



int main(int argc, char** argv) {
    int n;
    int proc;
    char procname[MPI_MAX_PROCESSOR_NAME];
    int len;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc);
    MPI_Get_processor_name(procname, &len);
    
    printf("proc %d, %s, reporting for duty\n", proc, procname);
    
    
    DAMPI_Init(proc, n);
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    
    if (!proc) {
      int a = 1;
      printf("I am proc %d, the value of my a is %d\n", proc, a);
      if (!setjmp(*info.env)) {
        MPI_Barrier(MPI_COMM_WORLD);
        printf("%d done it\n", proc);
        jmp_buf proc1buf;
        MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, info.envwin);
        printf("LOCKED\n");
        MPI_Get(&proc1buf, sizeof(jmp_buf), MPI_BYTE, 1, 0, sizeof(jmp_buf), MPI_BYTE, info.envwin);
        MPI_Win_unlock(1, info.envwin);
        printf("got the goods :(\n");
        MPI_Barrier(MPI_COMM_WORLD);
        longjmp(proc1buf, 1);
      }
      printf("I am proc %d, the value of my a is %d\n", proc, a);
    } else {
      int a = 2;
      printf("I am proc %d, the value of my a is %d\n", proc, a);
      if (!setjmp(*info.env)) {
        MPI_Barrier(MPI_COMM_WORLD);
        printf("%d done it\n", proc);
        jmp_buf proc0buf;
        MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, info.envwin);
        printf("LOCKED\n");
        MPI_Get(&proc0buf, sizeof(jmp_buf), MPI_BYTE, 0, 0, sizeof(jmp_buf), MPI_BYTE, info.envwin);
        MPI_Win_unlock(0, info.envwin);
        printf("got the goods :(\n");
        MPI_Barrier(MPI_COMM_WORLD);
        sleep(10);
        longjmp(proc0buf, 1);
      }
      printf("I am proc %d, the value of my a is %d\n", proc, a);
    }
    
    
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    DAMPI_Diag();
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
