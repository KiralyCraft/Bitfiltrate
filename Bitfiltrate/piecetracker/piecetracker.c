/*
 * piecetracker.c
 *
 *  Created on: Jun 7, 2023
 *      Author: kiraly
 */

#include "piecetracker.h"
#include <stdlib.h>
#include "dlinkedlist.h"

/*
 * Helper function that changes the operational status of this piece tracker, if certain conditions are met.
 *
 * Access to this function should be done in a synchronous manner, assuming to be locked.
 */
void _piecetracker_attemptOperational(piecetracker_t* __thePieceTracker)
{
	if (__thePieceTracker->blocksPerPiece == 0)
	{
		return;
	}
	if (__thePieceTracker->maximumPieceCount == 0)
	{
		return;
	}

	__thePieceTracker->thePieceTrackerStatus = PIECETRACKER_OPERATIONAL;
}
/*
 * Helper function which compares the "comparedStatus" against the given piece's status.
 */
uint8_t _piecetracker_pieceStateComparator(void* __givenPiece,void* __comparedStatus)
{
	piece_t* _thePiece = __givenPiece;
	piece_status_e _theStatus = *((piece_status_e*)__comparedStatus);

	if (_thePiece->thePieceStatus == _theStatus)
	{
		return 1;
	}
	return 0;
}
/*
 * Helper function that allows to function without locking.
 */
size_t _piecetracker_getUniversalPieceCount(piecetracker_t* __thePieceTracker,piece_status_e __desiredPieceStatus,uint8_t _shouldBlock)
{
	if (_shouldBlock) pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	size_t _toReturn = dlinkedlist_getCustomCount(__thePieceTracker->thePieces,_piecetracker_pieceStateComparator,&__desiredPieceStatus);
	if (_shouldBlock) pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return _toReturn;
}

piecetracker_t* piecetracker_constructTracker()
{
	//=== CONSTRUCTING THE TRACKER ===
	piecetracker_t* _constructedPieceTracker = malloc(sizeof(piecetracker_t));
	_constructedPieceTracker->thePieces = dlinkedlist_createList();
	_constructedPieceTracker->thePieceTrackerStatus = PIECETRACKER_UNINITIALIZED;
	_constructedPieceTracker->blocksPerPiece = 0;
	_constructedPieceTracker->maximumPieceCount = 0;

	pthread_mutex_init(&(_constructedPieceTracker->theSynchronizationMutex),NULL);

	return _constructedPieceTracker;
}
void piecetracker_setPieceSize(piecetracker_t* __thePieceTracker,size_t __log2PieceSize)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	//=== COMPUTING BLOCKS PER PIECE ===
	size_t _computedBlocksPerPiece = 1 << (__log2PieceSize - PIECE_MAX_BLOCK_SIZE);
	__thePieceTracker->blocksPerPiece = _computedBlocksPerPiece;

	_piecetracker_attemptOperational(__thePieceTracker);
	//TODO lock and signal accordingly
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
}

piecetracker_wishlist_t* piecetracker_getWish(piecetracker_t* __thePieceTracker)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));

	size_t _fullPieces = _piecetracker_getUniversalPieceCount(__thePieceTracker,PIECE_FULL,0);
	size_t _usedPieces = _piecetracker_getUniversalPieceCount(__thePieceTracker,PIECE_USED,0);

	size_t _maximumKnownPieceCount = __thePieceTracker->maximumPieceCount;

	//=== CHECKING IF THERE'S ANYTHING TO WISH FOR ===
	if (_fullPieces + _usedPieces == _maximumKnownPieceCount)
	{
		pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
		return NULL; //All pieces have been gathered, there's nothing to do.
	}

	//=== PREPARING TOOLS FOR COMPUTATION ===
	piecetracker_wishlist_t* _toReturn = malloc(sizeof(piecetracker_wishlist_t));
	size_t _currentStorageSize = dlinkedlist_getCount(__thePieceTracker->thePieces);

	//=== FILLING IN A PIECE WHICH WE DON'T HAVE AT ALL ===
	if (_currentStorageSize < _maximumKnownPieceCount)
	{
		_toReturn->wishedPieceIndex = _currentStorageSize; //The actual new piece's index is the exact amount we have now (since count = index + 1)
		_toReturn->wishedBlockIndex = 0;
		_toReturn->wishedBlockSize = (1 << PIECE_MAX_BLOCK_SIZE);

		pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
		return _toReturn;
	}

	//=== FINDING DESIRED TO-BE-FILLED BLOCK ===
	for (size_t _pieceIndexIterator = 0; _pieceIndexIterator < _currentStorageSize; _pieceIndexIterator++) //Iterate currently known pieces
	{
		piece_t* _thePieceInformation = dlinkedlist_getPosition(_pieceIndexIterator,__thePieceTracker->thePieces);
		piece_status_e _thePieceStatus = _thePieceInformation->thePieceStatus;

		if (_thePieceStatus == PIECE_EMPTY || _thePieceStatus == PIECE_FILLING) //If the iterated piece still has space
		{
			_toReturn->wishedPieceIndex = _pieceIndexIterator;
			_toReturn->wishedBlockIndex = _thePieceInformation->nextEmptyBlockIndex;
			_toReturn->wishedBlockSize = (1 << PIECE_MAX_BLOCK_SIZE);

			pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
			return _toReturn;
		}
	}

	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return NULL;
}

uint8_t piecetracker_ingestSwarmBlock(piecetracker_t* __thePieceTracker,swarm_block_t* __theProvidedBlock)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	size_t _thePieceIndex = __theProvidedBlock->thePieceIndex;
	size_t _thePieceOffset = __theProvidedBlock->theDataLength;
	size_t _theBlockLength = __theProvidedBlock->theDataLength;
	//=== BLOCK SIZE CHECKS ===
	if ((_theBlockLength >> PIECE_MAX_BLOCK_SIZE) != 1)
	{
		printf("DEBUG: Received block size that was different than expected\n");
		pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
		return 0;
	}
	//=== OFFSET CHECKS ===
	if (_thePieceOffset % (1 << PIECE_MAX_BLOCK_SIZE) != 0)
	{
		printf("DEBUG: Received block doesn't align properly inside the piece\n");
		pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
		return 0;
	}
	size_t _blockNumber = _thePieceOffset / (1 << PIECE_MAX_BLOCK_SIZE);

	printf("Received data for piece %d and block offset %d with size %d\n",_thePieceIndex,_blockNumber,_theBlockLength);

//	aici ai ramas si faceai un algoritm care face fill in dlinkedlist pana la piece-ul necesar,
//	apoi pregateste (daca e nevoie) campurile dinauntru la dimensiunea fixa de blocks-per-piece, si pune asta unde treubie.
//	apoi seteaza statusul de piece in functie de situatie.
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));

	return 1;
}

uint8_t piecetracker_setMaximumPieceCount(piecetracker_t* __thePieceTracker,size_t __assumedMaximumPieceCount)
{
	uint8_t _toReturn = 0;
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	if (__assumedMaximumPieceCount > __thePieceTracker->maximumPieceCount)
	{
		__thePieceTracker->maximumPieceCount = __assumedMaximumPieceCount;
		_toReturn = 1;

		_piecetracker_attemptOperational(__thePieceTracker);
	}
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return _toReturn;
}
size_t piecetracker_getMaximumPieceCount(piecetracker_t* __thePieceTracker)
{
	size_t _toReturn = 0;
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	_toReturn = __thePieceTracker->maximumPieceCount;
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return _toReturn;
}





size_t _piecetracker_getPieceCount(piecetracker_t* __thePieceTracker,piece_status_e __desiredPieceStatus)
{
	return _piecetracker_getUniversalPieceCount(__thePieceTracker,__desiredPieceStatus,1);
}

void piecetracker_destroyWish(piecetracker_wishlist_t* __theRuinedWish)
{
	free(__theRuinedWish);
}

piecetracker_status_e piecetracker_getCurrentStatus(piecetracker_t* __thePieceTracker)
{
	piecetracker_status_e _toReturn;
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	_toReturn = __thePieceTracker->thePieceTrackerStatus;
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return _toReturn;
}
