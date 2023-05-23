/*
 * dlinkedmap.c
 *
 *  Created on: May 21, 2023
 *      Author: kiraly
 */

#include "dlinkedmap.h"
#include "dlinkedlist.h"
#include <stdlib.h>

typedef struct
{
	size_t theIntegerKey;
	void* theKey;
	void* theValue;
} dlinkedmap_entry_t;

struct dlinkedmap_t
{
	dlinkedlist_t* underlyingList;
};

dlinkedmap_t* dlinkedmap_createMap()
{
	dlinkedmap_t* _theMap = malloc(sizeof(dlinkedmap_t));
	_theMap -> underlyingList = dlinkedlist_createList();
	return _theMap;
}
uint8_t dlinkedmap_destroy(dlinkedmap_t* __theList)
{
	while (dlinkedlist_getCount(__theList -> underlyingList) > 0)
	{
		//=== FETCH ENTITY ===
		dlinkedmap_entry_t* _firstEntity = dlinkedlist_getPosition(0, __theList -> underlyingList);
		free(_firstEntity);
		//=== CLEAR SLOT ===
		uint8_t _deletionResult = dlinkedlist_deletePosition(0, __theList -> underlyingList);

		if (_deletionResult != 1)
		{
			return 0;
		}
	}
	free(__theList);
	return 1;
}

/*
 * This is a helper function that returns 1 if the given key is equal to the key of the compared element.
 */
uint8_t _dlinkedmap_helper_keyComparison(void* __theKey, void* __comparedElement)
{
	dlinkedmap_entry_t* _comparedActualElement = __comparedElement;
	if (__theKey == _comparedActualElement -> theKey)
	{
		return 1;
	}
	return 0;
}

/*
 * This is a helper function that fetches the whole structure from the underlying linked list, instead of the value.
 */
dlinkedmap_entry_t* _dlinkedmap_getEntry(void* __theKey, dlinkedmap_t* __theMap)
{
	return dlinkedlist_getCustomElement(__theKey,_dlinkedmap_helper_keyComparison,__theMap->underlyingList);
}

uint8_t dlinkedmap_put(void* __theKey, void* __theValue, dlinkedmap_t* __theMap)
{
	dlinkedmap_entry_t* _foundEntry = _dlinkedmap_getEntry(__theKey,__theMap);

	if (_foundEntry != NULL)
	{
		_foundEntry -> theValue = __theValue;

		return 2;
	}
	else
	{
		dlinkedmap_entry_t* _newEntry = malloc(sizeof(dlinkedmap_entry_t));
		_newEntry -> theKey = __theKey;
		_newEntry -> theValue = __theValue;

		uint8_t _insertionResult = dlinkedlist_insertElement(_newEntry,__theMap -> underlyingList);

		if (_insertionResult == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

void* dlinkedmap_get(void* __theKey, dlinkedmap_t* __theMap)
{
	dlinkedmap_entry_t* _foundEntry = _dlinkedmap_getEntry(__theKey,__theMap);
	if (_foundEntry == NULL)
	{
		return NULL;
	}
	else
	{
		return _foundEntry -> theValue;
	}
}
uint8_t dlinkedmap_remove(void* __theKey, dlinkedmap_t* __theMap)
{
	dlinkedmap_entry_t* _foundEntry = _dlinkedmap_getEntry(__theKey,__theMap);
	if (_foundEntry == NULL)
	{
		return 0;
	}
	else
	{
		uint8_t _removalResult = dlinkedlist_deleteElement(_foundEntry,__theMap->underlyingList);
		if (!_removalResult)
		{
			return 0;
		}
		else
		{
			free(_foundEntry);
			return 1;
		}
	}
}
