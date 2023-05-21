/*
 * linkedlist.h
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#ifndef DLINKEDLIST_H_
#define DLINKEDLIST_H_

#include <stdint.h>

typedef struct dlinkedlist_element_t dlinkedlist_element_t;

struct dlinkedlist_element_t
{
	void* elementData;
	dlinkedlist_element_t* previousElement;
	dlinkedlist_element_t* nextElement;
};

typedef struct
{
	uint64_t elementCount;
	dlinkedlist_element_t* startElement;
	dlinkedlist_element_t* endElement;
} dlinkedlist_t;

//========GENESIS FUNCTIONS
dlinkedlist_t* dlinkedlist_createList();
/*
 * Attempts to destroy this list. If anything goes wrong, this function returns a value of "0". Otherwise it returns "1".
 */
uint8_t dlinkedlist_destroy(dlinkedlist_t* __theList);
//========OPERATIVE FUNCTIONS
/*
 * Attempts to insert the given element into the specified list, at the end of the list.
 * If somehow the insertion failed, this function will return a value of "0". Otherwise, it returns "1".
 */
uint8_t dlinkedlist_insertElement(void* __theElement, dlinkedlist_t* __theList);
/*
 * Attempts to delete the given element from the given list.
 * If the element is not found, or deletion failed for whatever other reason, this function will return a value of "0". Otherwise it returns "1".
 */
uint8_t dlinkedlist_deleteElement(void* __theElement, dlinkedlist_t* __theList);
/*
 * Attempts to delete the element at the given position. If this element does not exist, this function will return a value of "0". Otherwise it returns "1".
 */
uint8_t dlinkedlist_deletePosition(uint64_t __theElementPosition, dlinkedlist_t* __theList);
/*
 * Given a comparison criteria and a function for comparison, it fetches the whole element from the list that matches with the criteria, according to the function.
 * It returns NULL if no such element was found.
 *
 * For the comparison function, the first argument is always the provided criteria, and the second is the element within the list.
 */
void* dlinkedlist_getCustomElement(void* __comparisonCriteria,uint8_t (*__comparisonFunction)(void*,void*),dlinkedlist_t* __theList);
/*
 * Returns the number of elements currently present in the list provided.
 */
uint64_t dlinkedlist_getCount(dlinkedlist_t* __theList);
/*
 * Checks whether or not the given element exists in the specified list.
 * The comparison is simply a comparison of pointers.
 * It returns UINT64_MAX if the element was not found. Otherwise, it returns it's position.
 */
uint64_t dlinkedlist_checkExists(void* __theElement, dlinkedlist_t* __theList);
/*
 * The same as the normal function, but it also takes an argument to a function which actually does the comparison when attempting to find an element in the list.
 */
uint64_t dlinkedlist_checkCustomExists(void* __theElement, dlinkedlist_t* __theList, uint8_t (*__comparisonFunction)(void*,void*));


#endif /* DLINKEDLIST_H_ */
