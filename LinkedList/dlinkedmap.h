/*
 * dlinkedmap.h
 *
 *  Created on: May 21, 2023
 *      Author: kiraly
 */

#ifndef DLINKEDMAP_H_
#define DLINKEDMAP_H_

#include <stdint.h>

typedef struct dlinkedmap_t dlinkedmap_t;

//========GENESIS FUNCTIONS
dlinkedmap_t* dlinkedmap_createList();
/*
 * Attempts to destroy this list. If anything goes wrong, this function returns a value of "0". Otherwise it returns "1".
 */
uint8_t dlinkedmap_destroy(dlinkedmap_t* __theList);
//========OPERATIVE FUNCTIONS

/*
 * Inserts an element with the given key.
 * - If the key didn't exist before, this function returns 1 on success.
 * - If the key existed before and it was overwritten, this function returns 2.
 * - If insertion failed for whatever reason, it returns 0.
 */
uint8_t dlinkedmap_put(void* __theKey, void* __theValue, dlinkedmap_t* __theList);

/*
 * Returns an element from the list, identified by the given key. If the key doesn't exist, it returns NULL.
 * Otherwise it returns the actual value associated with the key.
 *
 * Note that this function might also return NULL if anything went wrong during the search.
 */
void* dlinkedmap_get(void* __theKey, dlinkedmap_t* __theList);

/*
 * Removes the given key (and therefore, the reference for the value) from the list.
 *
 * Returns 0 if anything failed, but returns 1 if everything went well.
 */
uint8_t dlinkedmap_remove(void* __theKey, dlinkedmap_t* __theList);

#endif /* DLINKEDMAP_H_ */
