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

	if (__theList -> elementCount == 0)
	{
		__theList -> startElement = _theNewElement;
		__theList -> endElement = _theNewElement;
	}
	else
	{
		__theList -> endElement -> nextElement = _theNewElement; //Currently final element's next is the new element
		__theList -> endElement = _theNewElement; //The new end element is the newly created one
	}

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
		if (_currentElement == NULL) //If we reached the end of the list
		{
			return 0;
		}
		else
		{
			if (__executionMode == 0) //Position mode
			{
				if (_currentPosition == *(uint64_t*)(__specifiedCriteria))
				{
					_foundElement = 1;
					break;
				}
			}
			else //Identity mode
			{
				if (_currentElement -> elementData == __specifiedCriteria)
				{
					_foundElement = 1;
					break;
				}
			}

			_currentElement = _currentElement -> nextElement;
			_currentPosition++;
		}
	}

	if (_foundElement)
	{
		//=======SPECIFIC CHECKS BASED ON WHICH ELEMENT WE FOUND==========
		if (_currentElement == __theList -> startElement)
		{
			dlinkedlist_element_t* _secondElement = _currentElement->nextElement;

			if (_secondElement != NULL) //If the list had at least two elements
			{
				_secondElement -> previousElement = NULL;
				__theList -> startElement = _secondElement;
			}
			else //If not, the list is now likely empty
			{
				__theList -> startElement = NULL;
			}
			__theList->elementCount--;

			free(_currentElement);
		}
		else if (_currentElement == __theList -> endElement)
		{
			dlinkedlist_element_t* _anteElement = _currentElement->previousElement;

			if (_anteElement != NULL) //If the list had at least two elements
			{
				_anteElement -> nextElement = NULL;
				__theList -> endElement = _anteElement;
			}
			else //If not, the list is now likely empty
			{
				__theList -> endElement = NULL;
			}
			__theList->elementCount--;

			free(_currentElement);
		}
		else
		{
			dlinkedlist_element_t* _furtherElement = _currentElement->nextElement;
			dlinkedlist_element_t* _anteElement = _currentElement->previousElement;

			_anteElement -> nextElement = _furtherElement;
			_furtherElement -> previousElement = _anteElement;

			__theList->elementCount--;
			free(_currentElement);
		}
		return 1;
	}
	else
	{
		return 0; //We couldn't find the element
	}
}

uint8_t dlinkedlist_deleteElement(void* __theElement, dlinkedlist_t* __theList)
{
	return _dlinkedlist_deleteGenericElement(__theElement,__theList,1);
}

uint8_t dlinkedlist_deletePosition(size_t __theElementPosition, dlinkedlist_t* __theList)
{
	return _dlinkedlist_deleteGenericElement(&__theElementPosition,__theList,0);
}

void* dlinkedlist_getCustomElement(void* __comparisonCriteria,uint8_t (*__comparisonFunction)(void*,void*),dlinkedlist_t* __theList)
{
	dlinkedlist_element_t* _currentElement = __theList -> startElement;

	uint64_t _currentPosition = 0;
	while(1)
	{
		if (_currentElement == NULL) //If we reached the end of the list
		{
			break;
		}
		else
		{
			if (__comparisonFunction(__comparisonCriteria,_currentElement -> elementData)) //If the comparison function returns a positive result
			{
				return _currentElement -> elementData;
			}
		}
		_currentElement = _currentElement -> nextElement;
		_currentPosition++;
	}
	return NULL;
}

void* dlinkedlist_getPosition(size_t __theElementPosition, dlinkedlist_t* __theList)
{
	if ()
}

uint64_t dlinkedlist_getCount(dlinkedlist_t* __theList)
{
	return __theList -> elementCount;
}

/*
 * Helper function to avoid duplicating code for checking if an element exists, depending on the comparison function.
 *
 * If a non-NULL comparison function is provided, the elements should be compared using that function.
 * Otherwise, simple pointer comparison will take place between elements.
 */
uint64_t _dlinkedlist_checkGenericExists(void* __theElement, dlinkedlist_t* __theList, uint8_t (*__comparisonFunction)(void*,void*))
{
	dlinkedlist_element_t* _currentElement = __theList -> startElement;

	uint64_t _currentPosition = 0;
	while(1)
	{
		if (_currentElement == NULL) //If we reached the end of the list
		{
			break;
		}
		else
		{
			if (__comparisonFunction == NULL)
			{
				if (_currentElement -> elementData == __theElement)
				{
					return _currentPosition;
				}
			}
			else
			{
				if (__comparisonFunction(_currentElement -> elementData,__theElement)) //If the comparison function returns a positive result
				{
					return _currentPosition;
				}
			}
			_currentElement = _currentElement -> nextElement;
			_currentPosition++;
		}
	}

	return UINT64_MAX;
}

uint64_t dlinkedlist_checkExists(void* __theElement, dlinkedlist_t* __theList)
{
	return _dlinkedlist_checkGenericExists(__theElement,__theList,NULL);
}

uint64_t dlinkedlist_checkCustomExists(void* __theElement, dlinkedlist_t* __theList, uint8_t (*__comparisonFunction)(void*,void*))
{
	return _dlinkedlist_checkGenericExists(__theElement,__theList,__comparisonFunction);
}


