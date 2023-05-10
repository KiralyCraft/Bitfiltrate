/*
 * linkedlist.c
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#include "dlinkedlist.h"
#include <stdlib.h>

dlinkedlist_t* dlinkedlist_createList()
{
	dlinkedlist_t* _theList = malloc(sizeof(dlinkedlist_t));

	_theList -> elementCount = 0;
	_theList -> endElement = NULL;
	_theList -> startElement = NULL;

	return _theList;
}

uint8_t dlinkedlist_destroy(dlinkedlist_t* __theList)
{
	while(dlinkedlist_getCount(__theList) > 0)
	{
		uint8_t _deletionResult = dlinkedlist_deletePosition(0,__theList);
		if (_deletionResult != 1)
		{
			return 0;
		}
		else
		{
			//All good :)
		}
	}

	free(__theList);
	return 1;
}


uint8_t dlinkedlist_insertElement(void* __theElement, dlinkedlist_t* __theList)
{
	dlinkedlist_element_t* _theNewElement = malloc(sizeof(dlinkedlist_element_t));

	_theNewElement -> elementData = __theElement;
	_theNewElement -> nextElement = NULL; //No element after this one
	_theNewElement -> previousElement = __theList -> endElement; //Link this element to the previous one (which is the final one)

	__theList -> endElement -> nextElement = _theNewElement; //Currently final element's next is the new element
	__theList -> endElement = _theNewElement; //The new end element is the newly created one

	__theList -> elementCount++;

	return 1;
}

/*
 * Helper function to avoid duplcating deletion code. This function can work both in multiple ways:
 * - Position mode
 * - Identity mode
 *
 * Which can be specified based upon the "exectionMode" argument:
 * - 0 - Position mode
 * - 1 - Identity mode
 */
uint8_t _dlinkedlist_deleteGenericElement(void* __specifiedCriteria, dlinkedlist_t* __theList, uint8_t __executionMode)
{
	//=======CASE SPECIFIC PRELIMIARY CHECKS FOR OPTIMISAITON==========
	if (__executionMode == 0)
	{
		if (*(uint64_t*)(__specifiedCriteria) >= dlinkedlist_getCount(__theList))
		{
			return 0;
		}
	}
	//========GENERAL BEHAVIOR=========
	dlinkedlist_element_t* _currentElement = __theList -> startElement;

	uint64_t _currentPosition = 0;
	uint8_t _foundElement = 0;

	while(1)
	{
		if (__executionMode == 0) //Position mode
		{
			if (_currentPosition == *(uint64_t*)(__specifiedCriteria))
			{
				break;
			}
		}
		else //Identity mode
		{
			if (_currentElement == __specifiedCriteria)
			{
				_foundElement = 1;
				break;
			}
		}

		_currentElement = _currentElement -> nextElement;
		_currentPosition++;
	}

	//=======SPECIFIC CHECKS BASED ON WHICH ELEMENT WE FOUND==========
	if (_currentElement == __theList -> startElement)
	{

	}

	return 1;
}

uint8_t dlinkedlist_deleteElement(void* __theElement, dlinkedlist_t* __theList)
{
	return _dlinkedlist_deleteGenericElement(__theElement,__theList,1);
}

uint8_t dlinkedlist_deletePosition(uint64_t __theElementPosition, dlinkedlist_t* __theList)
{
	return _dlinkedlist_deleteGenericElement(&__theElementPosition,__theList,0);
}




