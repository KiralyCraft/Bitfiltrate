/*
 * tcppeer.c
 *
 *  Created on: Jun 1, 2023
 *      Author: kiraly
 */

#include "tcppeer.h"
#include "dlinkedlist.h"

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <stdlib.h>

#define PIECES_PER_BYTE 8

/*
 * This is a helper function for adding a bitfield byte in a particular position.
 * The purpose of this method is such that it can be used both in thread safe environments, and unsafe ones.
 */
uint8_t _tcppeer_setBitfieldByteUnsafe(tcppeer_t* __thePeer, size_t __bitfieldByteIndex, uint8_t __bitfieldByte)
{
	uint8_t _resultToReturn;
	dlinkedlist_t* _bitfieldList = __thePeer -> peerBitfield;

	size_t _currentByteCount = dlinkedlist_getCount(_bitfieldList);
	if (__bitfieldByteIndex >= _currentByteCount)
	{
		//=== PREPARING THE BITFIELD BYTE ===
		uint8_t* _allocatedByte = malloc(sizeof(uint8_t));
		_allocatedByte[0] = __bitfieldByte;
		//=== INSERTING THE BYTE ===

		if (__bitfieldByteIndex > _currentByteCount)
		{
			uint8_t _localResult = 0;
			size_t _dummyElementsRequired = __bitfieldByteIndex - _currentByteCount;
			for (size_t _dummyIterator = 0; _dummyIterator < _dummyElementsRequired; _dummyIterator++)
			{
				uint8_t* _dummyByte = malloc(sizeof(uint8_t));
				_dummyByte[0] = 0;
				_localResult |= dlinkedlist_insertElement(_dummyByte,_bitfieldList);
			}
			_localResult |= dlinkedlist_insertElement(_allocatedByte,_bitfieldList);

			_resultToReturn = _localResult;
		}
		else //If the desired index is precisely how many elements we have, we can simply add add at the end.
		{
			_resultToReturn = dlinkedlist_insertElement(_allocatedByte,_bitfieldList); //Inserting at the end
		}
	}
	else
	{
		uint8_t* _existingByte = dlinkedlist_getPosition(__bitfieldByteIndex, _bitfieldList);
		_existingByte[0] = __bitfieldByte;
	}
	return _resultToReturn;
}
uint8_t tcppeer_setBitfieldByte(tcppeer_t* __thePeer, size_t __bitfieldByteIndex, uint8_t __bitfieldByte)
{
	uint8_t _resultToReturn;
	pthread_mutex_lock(&(__thePeer->bitfieldMutex));
	_resultToReturn = _tcppeer_setBitfieldByteUnsafe(__thePeer,__bitfieldByteIndex,__bitfieldByte);
	pthread_mutex_unlock(&(__thePeer->bitfieldMutex));
	return _resultToReturn;
}

uint8_t tcppeer_hasPiece(tcppeer_t* __thePeer,size_t __thePieceIndex)
{
	uint8_t _resultToReturn;
	pthread_mutex_lock(&(__thePeer->bitfieldMutex));
	dlinkedlist_t* _bitfieldList = __thePeer -> peerBitfield;
	size_t _currentByteCount = dlinkedlist_getCount(_bitfieldList);

	size_t _pieceBytePosition = __thePieceIndex / PIECES_PER_BYTE;
	if (_pieceBytePosition >= _currentByteCount) //Requested piece is out of bounds
	{
		_resultToReturn = 0;
	}
	else
	{
		uint8_t _pieceByteDataCopy = *((uint8_t*)dlinkedlist_getPosition(_pieceBytePosition, _bitfieldList)); //No need to check for NULL, since we've checked the counts
		uint8_t _shiftsLeft = __thePieceIndex % PIECES_PER_BYTE;

		_pieceByteDataCopy >>= (8 - (_shiftsLeft + 1));

		_resultToReturn = _pieceByteDataCopy & 0x1;
	}
	pthread_mutex_unlock(&(__thePeer->bitfieldMutex));
	return _resultToReturn;
}
uint8_t tcppeer_setPiece(tcppeer_t* __thePeer,size_t __thePieceIndex,uint8_t __hasPiece)
{
	uint8_t _resultToReturn;
	pthread_mutex_lock(&(__thePeer->bitfieldMutex));
	dlinkedlist_t* _bitfieldList = __thePeer -> peerBitfield;
	size_t _currentByteCount = dlinkedlist_getCount(_bitfieldList);

	size_t _pieceBytePosition = __thePieceIndex / PIECES_PER_BYTE;

	int8_t _intendedShifts = __thePieceIndex % PIECES_PER_BYTE;
	uint8_t _pieceByteMask = 0x1 << (8 - (_intendedShifts+1));

	if (_pieceBytePosition >= _currentByteCount) //Requested piece is out of bounds, so we can safely set our mask
	{
		_resultToReturn = _tcppeer_setBitfieldByteUnsafe(__thePeer,_pieceBytePosition,_pieceByteMask);
	}
	else
	{
		uint8_t _pieceByteDataCopy = *((uint8_t*)dlinkedlist_getPosition(_pieceBytePosition, _bitfieldList)); //No need to check for NULL, since we've checked the counts
		uint8_t _pieceByteDataMasked = _pieceByteDataCopy | _pieceByteMask;
		_resultToReturn = _tcppeer_setBitfieldByteUnsafe(__thePeer,_pieceBytePosition,_pieceByteDataMasked);

	}
	pthread_mutex_unlock(&(__thePeer->bitfieldMutex));
	return _resultToReturn;
}
