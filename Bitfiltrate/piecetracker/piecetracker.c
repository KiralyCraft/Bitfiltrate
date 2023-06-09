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
	_constructedPieceTracker->usedPieces = 0;


	pthread_mutex_init(&(_constructedPieceTracker->theSynchronizationMutex),NULL);
	pthread_cond_init(&(_constructedPieceTracker->theExportConditionalVariable),NULL);
	return _constructedPieceTracker;
}
void piecetracker_setPieceSize(piecetracker_t* __thePieceTracker,size_t __log2PieceSize)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	//=== COMPUTING BLOCKS PER PIECE ===
	size_t _computedBlocksPerPiece = 1 << (__log2PieceSize - PIECE_MAX_BLOCK_SIZE);
	__thePieceTracker->blocksPerPiece = _computedBlocksPerPiece;
	__thePieceTracker->pieceSize = __log2PieceSize;

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
		_toReturn->wishedBlockIndexStart = 0;
		_toReturn->wishedBlockIndexEnd = __thePieceTracker->blocksPerPiece - 1;
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

			uint8_t _foundEmptyBlock = 0;
			if (_thePieceInformation->filledBlocks < __thePieceTracker->blocksPerPiece) //Checking if we should have space
			{
				for (size_t _blockIndexIterator = 0; _blockIndexIterator < __thePieceTracker->blocksPerPiece; _blockIndexIterator++)
				{
					if (_thePieceInformation->theBlockFillment[_blockIndexIterator] == 0)
					{
						if (_foundEmptyBlock == 0) //If we didn't find any block yet
						{
							_toReturn->wishedBlockIndexStart = _blockIndexIterator;
							_toReturn->wishedBlockIndexEnd = _blockIndexIterator;
							_foundEmptyBlock = 1;
						}
						else //If there are more empty blocks, even though we already found one
						{
							_toReturn->wishedBlockIndexEnd = _blockIndexIterator;
						}
					}
				}
			}

			_toReturn->wishedBlockSize = (1 << PIECE_MAX_BLOCK_SIZE);

			if (_foundEmptyBlock == 0)
			{
				printf("DEBUG: Could not find empty block in piece, despite being theoretically empty\n");
				//TODO handle situation
			}

			pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
			return _toReturn;
		}
	}

	free(_toReturn); //Free the allocation in case of failure
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return NULL;
}

/*
 * Helper function which generates new, empty piece data.
 */
piece_t* _piecetracker_generateNewPiece(size_t __blocksPerPiece)
{
	piece_t* _toReturn = malloc(sizeof(piece_t));
	_toReturn -> filledBlocks = 0;
	_toReturn -> theBlockFillment = calloc(__blocksPerPiece,sizeof(uint8_t));
	_toReturn -> theBlocks = malloc(sizeof(uint8_t*) * __blocksPerPiece);
	_toReturn -> thePieceStatus = PIECE_EMPTY;

	return _toReturn;
}

/*
 * This method is expected to reuse the data represented in the provided within the block, yet at the end of execution
 * the wrapper is destroyed.
 */
uint8_t piecetracker_ingestSwarmBlock(piecetracker_t* __thePieceTracker,swarm_block_t* __theProvidedBlock)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	size_t _thePieceIndex = __theProvidedBlock->thePieceIndex;
	size_t _thePieceOffset = __theProvidedBlock->theOffsetWithinPiece;
	size_t _theBlockLength = __theProvidedBlock->theDataLength;
	uint8_t* _theActualData = __theProvidedBlock->theData;

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

//	printf("Received data for piece %d and block offset %d with size %d\n",_thePieceIndex,_blockNumber,_theBlockLength);
	//=== INSERTING NEW PIECES IF NEEDED ===
	piece_t* _newlyInsertedDesiredPiece = NULL; //In the event that the piece is exactly what we require anyway, keep track of it to avoid reiterating

	size_t _currentStorageSize = dlinkedlist_getCount(__thePieceTracker->thePieces);
	if (_thePieceIndex >= _currentStorageSize)
	{
		size_t _requiredNewPieces = (_thePieceIndex - _currentStorageSize)+1;
		for (size_t _newPieceIterator = 0; _newPieceIterator < _requiredNewPieces; _newPieceIterator++)
		{
			piece_t* _theNewPiece = _piecetracker_generateNewPiece(__thePieceTracker->blocksPerPiece);
			uint8_t _insertionResult = dlinkedlist_insertElement(_theNewPiece,__thePieceTracker->thePieces);
			if (_insertionResult == 0)
			{
				printf("DEBUG: Could not insert the new piece in piecetracker\n");
				; //TODO handle exception, clean the newly created piece, return 0 and unlock
			}

			if (_thePieceIndex == (_currentStorageSize + _newPieceIterator - 1))
			{
				_newlyInsertedDesiredPiece = _theNewPiece;
			}
		}
	}

	//=== FETCHING THE PIECE DATA ===
	piece_t* _targetPiece;
	if (_newlyInsertedDesiredPiece != NULL)
	{
		_targetPiece = _newlyInsertedDesiredPiece;
	}
	else
	{
		_targetPiece = dlinkedlist_getPosition(_thePieceIndex,__thePieceTracker->thePieces);
	}

	//=== FILLING THE BLOCK IF POSSIBLE ===

	if (_targetPiece->thePieceStatus == PIECE_EMPTY || _targetPiece->thePieceStatus == PIECE_FILLING)
	{
		uint8_t _isBlockFilledAlready = _targetPiece->theBlockFillment[_blockNumber];

		if (_isBlockFilledAlready)
		{
			printf("Dropping already filled data for piece %lu with block %lu. Currently has %lu pieces out of %lu\n",_thePieceIndex,_blockNumber,_targetPiece->filledBlocks,__thePieceTracker->blocksPerPiece);
			//Block is already full, simply clear duplicate data and proceed (since this is not going to be added and not cleaned either)
			free(__theProvidedBlock->theData);
			pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
			return 1;
		}
		else
		{
			printf("Filling data for piece %lu with block %lu. Currently has %lu pieces out of %lu\n",_thePieceIndex,_blockNumber,_targetPiece->filledBlocks,__thePieceTracker->blocksPerPiece);
			_targetPiece->theBlocks[_blockNumber] = _theActualData; //This is safe to do, because the data does not get freed after exiting this function
			_targetPiece->theBlockFillment[_blockNumber] = 1; //Mark that we filled this block

			_targetPiece->filledBlocks++; //Increment the number of blocks filled

			if (_targetPiece -> thePieceStatus == PIECE_EMPTY)
			{
				_targetPiece-> thePieceStatus = PIECE_FILLING;
			}

			if (_targetPiece->filledBlocks == __thePieceTracker->blocksPerPiece) //If by adding this block, the piece becomes complete
			{
				_targetPiece->thePieceStatus = PIECE_FULL;
				pthread_cond_broadcast(&(__thePieceTracker->theExportConditionalVariable)); //Notify any exporters which may be waiting
			}

			pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
			return 1;
		}
	}

//	aici ai ramas si faceai un algoritm care face fill in dlinkedlist pana la piece-ul necesar,
//	apoi pregateste (daca e nevoie) campurile dinauntru la dimensiunea fixa de blocks-per-piece, si pune asta unde treubie.
//	apoi seteaza statusul de piece in functie de situatie.
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));

	return 0;
}

uint8_t piecetracker_isWishFulfilled(piecetracker_t* __thePieceTracker,piecetracker_wishlist_t* __theWish)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	size_t _currentStorageSize = dlinkedlist_getCount(__thePieceTracker->thePieces);

	if (__theWish -> wishedPieceIndex >= _currentStorageSize) //Wished piece is out of bounds
	{
		pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
		return 0;
	}

	size_t _wishedBlockIndexStart = __theWish->wishedBlockIndexStart;
	size_t _wishedBlockIndexEnd = __theWish->wishedBlockIndexEnd;
	piece_t* _theWishedPiece = dlinkedlist_getPosition(__theWish -> wishedPieceIndex,__thePieceTracker->thePieces);

	for (size_t _blockIndexIterator = _wishedBlockIndexStart; _blockIndexIterator <= _wishedBlockIndexEnd; _blockIndexIterator++)
	{
		if (_theWishedPiece->theBlockFillment[_blockIndexIterator] == 0) //If any block within the wished range is not fulfilled
		{
			pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
			return 0;
		}
	}

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

uint8_t piecetracker_areAllPiecesUsed(piecetracker_t* __thePieceTracker)
{
	uint8_t _areAllPiecesUsed = 0;
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	if (__thePieceTracker->usedPieces == __thePieceTracker->maximumPieceCount)
	{
		_areAllPiecesUsed = 1;
	}
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return _areAllPiecesUsed;
}

piecetracker_pieceexport_t* piecetracker_consumeExportPieceIfAny(piecetracker_t* __thePieceTracker)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	pthread_cond_wait(&(__thePieceTracker->theExportConditionalVariable),&(__thePieceTracker->theSynchronizationMutex));
	size_t _currentPieceCount = dlinkedlist_getCount(__thePieceTracker->thePieces);
	if (_currentPieceCount == 0)
	{
		pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
		return NULL;
	}

	for (size_t _pieceIterator = 0; _pieceIterator < _currentPieceCount; _pieceIterator++)
	{
		piece_t* _thePiece = dlinkedlist_getPosition(_pieceIterator,__thePieceTracker->thePieces);
		if (_thePiece -> thePieceStatus == PIECE_FULL)
		{
			__thePieceTracker -> usedPieces++; //Increment the number of used pieces, the redundant helper variable.
			_thePiece -> thePieceStatus = PIECE_USED;

			//=== FILLING THE EXPORT WRAPPER ===
			piecetracker_pieceexport_t* _theExportWrapper = malloc(sizeof(piecetracker_pieceexport_t));
			_theExportWrapper -> theExportedPiece = _thePiece;
			_theExportWrapper -> pieceIndex = _pieceIterator;
			_theExportWrapper -> pieceSizeLog2 = __thePieceTracker->pieceSize;
			_theExportWrapper -> blockSizeLog2 = PIECE_MAX_BLOCK_SIZE;


			pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
			return _theExportWrapper;
		}
	}
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return NULL;
}

void piecetracker_destroyExportPieceWrapper(piecetracker_pieceexport_t* __theExportedPiece)
{
	free(__theExportedPiece); //Only destroy the wrapper, as the pointer inside is still used, do not free that
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
