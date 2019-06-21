#ifndef MIGRATE_H
#define MIGRATE_H

#include "profile.h"

#define SWAP_THRESHOLD 10


/*  compute the value of the configuration represented by this table
 *
 */
double value (struct BNodeTable * bt, int* rankprocs);

int should_migrate(struct BNodeTable *bt);

#endif
