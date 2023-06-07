/*
 * piecetracker.h
 *
 *  Created on: Jun 7, 2023
 *      Author: kiraly
 */

#ifndef PIECETRACKER_PIECETRACKER_H_
#define PIECETRACKER_PIECETRACKER_H_

#include "dlinkedlist.h"
#include "../swarm/swarm_block.h"
#include <pthread.h>

/*
 * Define the maximum (expressed as log2) block size.
 *
 * Looks like a safe bet would be 16 KB (2^14 bytes)
 */
#define PIECE_MAX_BLOCK_SIZE 14

typedef enum
{
	/*
	 * The status of a piece which has been used and had it's contents therefore discarded.
	 */
	PIECE_USED,
	/*
	 * The piece is full and ready to be used.
	 */
	PIECE_FULL,
	/*
	 * The piece is not empty, and has had some activity at some point.
	 */
	PIECE_FILLING,
	/*
	 * The piece is empty is pending filling.
	 */
	PIECE_EMPTY
} piece_status_e;

typedef enum
{
	/*
	 * The status which the piece tracker has when it is first created, but not initialized with the number of blocks per piece.
	 */
	PIECETRACKER_UNINITIALIZED,
	/*
	 * The status which the piece tracker is set to after the piece size is known.
	 */
	PIECETRACKER_OPERATIONAL
} piecetracker_status_e;

typedef struct
{
	uint8_t** theBlocks;
	piece_status_e thePieceStatus;
} piece_t;

typedef struct
{
	//=== STRUCTURAL ASPECTS ===
	size_t blocksPerPiece;
	dlinkedlist_t* thePieces;
	piecetracker_status_e thePieceTrackerStatus;
	//=== SYNCHRONIZAITON ===
	pthread_mutex_t theSynchronizationMutex;
	//TODO add conditional variable that notifies external watchers when this tracker becomes operational
} piecetracker_t;

piecetracker_t* piecetracker_constructTracker();
/*
 * Setting the piece size of the tracker should be the first thing done before actually using this piece tracker.
 */
void piecetracker_setPieceSize(piecetracker_t* __thePieceTracker,size_t __log2PieceSize);
uint8_t piecetracker_ingestSwarmBlock(piecetracker_t* __thePieceTracker,swarm_block_t* __theProvidedBlock);
size_t piecetracker_getPieceCount(piecetracker_t* __thePieceTracker,piece_status_e __desiredPieceStatus);

#endif /* PIECETRACKER_PIECETRACKER_H_ */
