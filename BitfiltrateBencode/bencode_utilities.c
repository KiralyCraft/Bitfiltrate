/*
 * bencode_utilities.c
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#include "bencode_utilities.h"
#include <string.h>

bencode_et* bencode_readStringDictKey(char* __dictionaryKey, bencode_et* __theDictionary)
{
	bencode_et_dictionary* _dictionaryData = __theDictionary->bencodeSubData;
	uint64_t _dictionaryLength = _dictionaryData->dictionaryElementCount;
	for (uint64_t _dictionaryIterator = 0; _dictionaryIterator < _dictionaryLength; _dictionaryIterator++)
	{
		bencode_et_dictionary_pair* _iteratedPair = _dictionaryData->dictionaryContents[_dictionaryIterator];
		bencode_et_bytestring* _iteratedKey = _iteratedPair->dictionaryKey->bencodeSubData;

		if (strcmp(__dictionaryKey,_iteratedKey->byteStringData) == 0)
		{
			return _iteratedPair->dictionaryValue;
		}
	}

	return NULL;
}


