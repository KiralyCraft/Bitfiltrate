/*
 * bencode_internal.c
 *
 *  Created on: Apr 8, 2023
 *      Author: kiraly
 */

#include "bencode_internal.h"
#include <stdlib.h>
#include <string.h>

uint8_t _bencode_readData(bencode_t* __providedBencode,uint8_t __shouldIncrement)
{
	uint8_t _toReturn = ((uint8_t*)__providedBencode->memoryRegion)[__providedBencode->currentOffset];
	if (__shouldIncrement)
	{
		__providedBencode->currentOffset++;
	}
	return _toReturn;
}

void _bencode_incrementOffset(bencode_t* __providedBencode)
{
	__providedBencode->currentOffset++;
}

bencode_e bencode_probeType(bencode_t* __providedBencode)
{
	uint8_t _currentToken = _bencode_readData(__providedBencode,0);

	switch (_currentToken)
	{
		case 'i':
			return BENCODE_INTEGER;
		case 'l':
			return BENCODE_LIST;
		case 'd':
			return BENCODE_DICTIONARY;
		case 'e':
			return BENCODE_TERMINATOR;
		case ':':
			return BENCODE_SEPARATOR;
	}
	return BENCODE_BYTE_STRING;
}

/**
 * Utility function to read an Integer from where it actually begins, past prefixes.
 *
 * After execution, this function would've moved past the terminators, be them 'e' or ':' in case of byte strings.
 */
int64_t _bencode_readInteger(bencode_t* __providedBencode)
{
	int64_t _readInteger = 0;
	uint8_t _isSigned = 0;

	uint8_t _signCheck = _bencode_readData(__providedBencode,0); //Check without moving if the number has a sign or not
	if (_signCheck == '-')
	{
		_isSigned = 1;
		_bencode_incrementOffset(__providedBencode); //If it does, move so the number fetching routine reads it right
	}

	while(bencode_probeType(__providedBencode) != BENCODE_TERMINATOR && bencode_probeType(__providedBencode) != BENCODE_SEPARATOR)
	{
		uint8_t _readEntry = _bencode_readData(__providedBencode,1);
		_readInteger = (_readEntry-'0') + _readInteger*10;
	}
	_bencode_incrementOffset(__providedBencode); //Increment past the terminator

	if (_isSigned)
	{
		_readInteger = _readInteger * -1;
	}

	return _readInteger;
}

/**
 * Utility function to read N characters from the bencode. The returned representation will be one byte larger, to accomodate a NULL charcater.
 *
 * After execution, this function would've moved past the required number of characters.
 */
char* _bencode_readString(bencode_t* __providedBencode,uint64_t __byteStringLength)
{
	char* _allocatedString = malloc(__byteStringLength+1);
	memcpy(_allocatedString,__providedBencode->memoryRegion + __providedBencode->currentOffset,__byteStringLength);
	_allocatedString[__byteStringLength] = 0;
	__providedBencode->currentOffset = __providedBencode->currentOffset + __byteStringLength;
	return _allocatedString;
}

bencode_et* bencode_readNext(bencode_t* __providedBencode)
{
	bencode_e _nextTokenType = bencode_probeType(__providedBencode);

	bencode_et* _toReturn = malloc(sizeof(bencode_et));
	_toReturn->bencodeType = _nextTokenType;

	if (_nextTokenType == BENCODE_INTEGER)
	{
		_bencode_incrementOffset(__providedBencode);
		int64_t _readInteger = _bencode_readInteger(__providedBencode);

		bencode_et_integer* _containedData = malloc(sizeof(bencode_et_integer));
		_containedData->integerData = _readInteger;
		_toReturn->bencodeSubData = _containedData;
	}
	else if (_nextTokenType == BENCODE_BYTE_STRING)
	{
		int64_t _byteStringLength = _bencode_readInteger(__providedBencode);

		bencode_et_bytestring* _containedData = malloc(sizeof(bencode_et_bytestring));
		_containedData->byteStringData = _bencode_readString(__providedBencode,_byteStringLength);
		_containedData->byteStringDataLength = _byteStringLength+1; //Include the NULL terminator

		_toReturn->bencodeSubData = _containedData;
	}
	return _toReturn;
}


