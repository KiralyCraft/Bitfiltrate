/*
 * dlinkedmap.c
 *
 *  Created on: May 21, 2023
 *      Author: kiraly
 */

#include "dlinkedmap.h"
#include "dlinkedlist.h"
#include <stdlib.h>

struct dlinkedmap_t
{
	dlinkedlist_t* underlyingList;
};

dlinkedmap_t* dlinkedmap_createList()
{
	dlinkedmap_t* _theMap = malloc(sizeof(dlinkedmap_t));
	_theMap -> underlyingList = dlinkedlist_createList();
	return _theMap;
}
uint8_t dlinkedmap_destroy(dlinkedmap_t* __theList)
{

}
uint8_t dlinkedmap_put(void* __theKey, void* __theValue, dlinkedmap_t* __theList)
{

}
void* dlinkedmap_get(void* __theKey, dlinkedmap_t* __theList)
{

}
uint8_t dlinkedmap_remove(void* __theKey, dlinkedmap_t* __theList)
{

}
