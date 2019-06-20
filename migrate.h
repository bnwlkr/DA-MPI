#ifndef MIGRATE_H
#define MIGRATE_H

#include "profile.h"

#define SWAP_THRESHOLD 10

struct DAMPI_Suitcase {
  int size;
  void * data;
};


/*  processes regularly enter this function to see if a migration is necessary. If they decide that they should move to another proc, 
 *  they will post that information at the bnode and wait for synchronization.
 *
 */
int airlock();

/*  get the bnode's table of migration and frequency information 
 */

void get_bt(struct BNodeTable * bt);

/*  compute the value of the configuration represented by this table
 *
 */
double value (struct BNodeTable * bt, int* rankprocs);

int should_migrate(struct BNodeTable *bt);

#endif
