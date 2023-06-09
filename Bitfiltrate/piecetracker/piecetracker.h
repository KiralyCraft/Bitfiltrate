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
	size_t wishedBlockIndexStart;
	size_t wishedBlockIndexEnd;
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
	size_t filledBlocks;
	uint8_t* theBlockFillment;
	uint8_t** theBlocks;
	piece_status_e thePieceStatus;
} piece_t;

/*
 * This structure is exported whenever a piece is consumed, such that the one who uses it gets all
 * information in one shot.
 */
typedef struct
{
	piece_t* theExportedPiece;
	size_t pieceIndex;
	size_t pieceSizeLog2;
	size_t blockSizeLog2;
} piecetracker_pieceexport_t;

typedef struct
{
	//=== REDUNDANT HELPERS ===
	/*
	 * Counts the number of used pieces, as they get used.
	 * This is a redundant variable, as the same information can be found by counting the list, but that
	 * is significantly slower.
	 */
	size_t usedPieces;
	/*
	 * The piece size can be computed based on the blocks per piece value, and the size of a block.
	 * This method, however, is a bit faster than computing the multiplication every time.
	 */
	size_t pieceSize;
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
	pthread_cond_t theExportConditionalVariable;
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
 * This method is a faster way of checking whether the current number of used pieces equals
 * the maximum number of pieces indicated.
 */
uint8_t piecetracker_areAllPiecesUsed(piecetracker_t* __thePieceTracker);

/*
 * If there exists a piece which is FULL and therefore pending to be flushed to the disk,
 * this function returns it and sets it's status as "USED" before returning it.
 *
 * It is expected that whoever calls this function to also free the memory used
 * by the actual data in the block, but otherwise leave it intact.
 *
 * It is also expected for the caller of this function to also destroy the wrapper.
 *
 * This method BLOCKS UNTIL PIECES ARE AVAILABLE!
 *
 * If no such piece is available, this function should return NULL.
 */
piecetracker_pieceexport_t* piecetracker_consumeExportPieceIfAny(piecetracker_t* __thePieceTracker);
void piecetracker_destroyExportPieceWrapper(piecetracker_pieceexport_t* __theExportedPiece);

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
uint8_t piecetracker_isWishFulfilled(piecetracker_t* __thePieceTracker,piecetracker_wishlist_t* __theWish);

piecetracker_status_e piecetracker_getCurrentStatus(piecetracker_t* __thePieceTracker);


#endif /* PIECETRACKER_PIECETRACKER_H_ */
