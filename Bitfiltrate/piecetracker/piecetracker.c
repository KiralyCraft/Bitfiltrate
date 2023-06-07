/*
 * piecetracker.c
 *
 *  Created on: Jun 7, 2023
 *      Author: kiraly
 */

#include "piecetracker.h"
#include <stdlib.h>

piecetracker_t* piecetracker_constructTracker()
{
	//=== CONSTRUCTING THE TRACKER ===
	piecetracker_t* _constructedPieceTracker = malloc(sizeof(piecetracker_t));
	_constructedPieceTracker->thePieces = dlinkedlist_createList();
	_constructedPieceTracker->thePieceTrackerStatus = PIECETRACKER_UNINITIALIZED;

	pthread_mutex_init(&(_constructedPieceTracker->theSynchronizationMutex),NULL);

	return _constructedPieceTracker;
}
void piecetracker_setPieceSize(piecetracker_t* __thePieceTracker,size_t __log2PieceSize)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	//=== COMPUTING BLOCKS PER PIECE ===
	size_t _computedBlocksPerPiece = 1 << (__log2PieceSize - PIECE_MAX_BLOCK_SIZE);
	__thePieceTracker->blocksPerPiece = _computedBlocksPerPiece;
	__thePieceTracker->thePieceTrackerStatus = PIECETRACKER_OPERATIONAL;
	//TODO lock and signal accordingly
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
}

uint8_t piecetracker_ingestSwarmBlock(piecetracker_t* __thePieceTracker,swarm_block_t* __theProvidedBlock)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	aici ai ramas si faceai un algoritm care face fill in dlinkedlist pana la piece-ul necesar,
	apoi pregateste (daca e nevoie) campurile dinauntru la dimensiunea fixa de blocks-per-piece, si pune asta unde treubie.
	apoi seteaza statusul de piece in functie de situatie.
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
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
size_t piecetracker_getPieceCount(piecetracker_t* __thePieceTracker,piece_status_e __desiredPieceStatus)
{
	pthread_mutex_lock(&(__thePieceTracker->theSynchronizationMutex));
	size_t _toReturn = dlinkedlist_getCustomCount(__thePieceTracker->thePieces,_piecetracker_pieceStateComparator,&__desiredPieceStatus);
	pthread_mutex_unlock(&(__thePieceTracker->theSynchronizationMutex));
	return _toReturn;
}
