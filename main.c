#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include "dampi.h"
#include <ucontext.h>

#define STACK_SIZE (1<<15)

/*  potential solution: every function called by an MPI process can do so using a calling helper that will actually make a context (with a stack) for the called function. This stack can then be the thing that is passed between processes during migration. The stack just needs to be reassigned to the ucontext on the other side. just need to pas stack contents and the index of the function to the other proc
 */



char foostack[STACK_SIZE];
char barstack[STACK_SIZE];

void foo();
void bar();

void foo () {
  printf("%d executing foo\n", info.proc);
  ucontext_t fooc;
  getcontext(&fooc);
  fooc.uc_stack.ss_sp = foostack;
  fooc.uc_stack.ss_size = STACK_SIZE;
  makecontext(&fooc, bar, 0);
  setcontext(&fooc);

}

void bar () {
  printf("%d executing bar\n", info.proc);
  ucontext_t barc;
  getcontext(&barc);
  barc.uc_stack.ss_sp = barstack;
  barc.uc_stack.ss_size = STACK_SIZE;
  makecontext(&barc, foo, 0);
  setcontext(&barc);
}



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
  
    DAMPI_Send(NULL, 0, MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    
    
//    if (proc)
//      foo ();
//    else
//      bar();
    
    
    
    
    
    
    MPI_Barrier(MPI_COMM_WORLD);
  
  
    DAMPI_Diag();
    
    
    DAMPI_Finalize();
    MPI_Finalize(); 
    return 0;
}
