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

typedef struct
{
	size_t wishedPieceIndex;
	size_t wishedBlockIndex;
	/*
	 * The block size is expressed normally, not as a logarithm of two.
	 */
	size_t wishedBlockSize;
} piecetracker_wishlist_t;

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
	 * The status which the piece tracker is set to after the piece size is known, and the maximum piece size is not zero.
	 */
	PIECETRACKER_OPERATIONAL
} piecetracker_status_e;

typedef struct
{
	uint8_t** theBlocks;
	piece_status_e thePieceStatus;
	/*
	 * This variable keeps track which block should be next filled for this given piece, to avoid checking.
	 */
	size_t nextEmptyBlockIndex;
} piece_t;

typedef struct
{
	//=== STRUCTURAL ASPECTS ===
	/*
	 * This filed indicates the number of blocks per piece, according to the piece size and
	 * the statically defined maximum block size.
	 */
	size_t blocksPerPiece;
	/*
	 * A storage list containing internal representations of the piece data (and the internal blocks)
	 */
	dlinkedlist_t* thePieces;
	/*
	 * The current status of the piece tracker. Initially, the tracker is uninitialized as it does not have
	 * the blocks per piece value set.
	 */
	piecetracker_status_e thePieceTrackerStatus;

	/*
	 * The maximum piece count, according to scraped peer information.
	 */
	size_t maximumPieceCount;
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

/*
 * Allows an external entity to specify what the maximum number of pieces currently is, as reported by the swarm.
 * This method will only effectively change the current maximum piece count if the provided value is larger than
 * the current one.
 *
 * If this method didn't change anything, it returns 0. Otherwise it returns 1.
 */
uint8_t piecetracker_setMaximumPieceCount(piecetracker_t* __thePieceTracker, size_t __assumedMaximumPieceCount);
size_t piecetracker_getMaximumPieceCount(piecetracker_t* __thePieceTracker);

/*
 * Returns information about what block the piece tracker requests.
 *
 * The bundled information is supposed to be cleared by the one who requests it.
 *
 * If no wish can be supplied, this function returns NULL.
 */
piecetracker_wishlist_t* piecetracker_getWish(piecetracker_t* __thePieceTracker);
void piecetracker_destroyWish(piecetracker_wishlist_t* __theRuinedWish);

piecetracker_status_e piecetracker_getCurrentStatus(piecetracker_t* __thePieceTracker);

#endif /* PIECETRACKER_PIECETRACKER_H_ */
