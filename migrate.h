#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>





/*  processes regularly enter this function to see if a migration is necessary. If they decide that they should move to another proc, they will post that information at the bnode.
 *
 */
int DAMPI_Airlock();



