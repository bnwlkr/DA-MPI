//
//  profile.c
//  
//
//  Created by Ben Walker on 2019-06-13.
//

#include "profile.h"

static int totalBytes = 0;
static double totalTime = 0.0; 
 
int MPI_Send(const void* buffer, int count, MPI_Datatype datatype, 
             int dest, int tag, MPI_Comm comm) {
  
   double tstart = MPI_Wtime();      
   int size; 
   int result    = PMPI_Send(buffer,count,datatype,dest,tag,comm);    
 
   totalTime  += MPI_Wtime() - tstart;         
 
   MPI_Type_size(datatype, &size);  
   totalBytes += count*size; 
 
   return result;                        
} 
