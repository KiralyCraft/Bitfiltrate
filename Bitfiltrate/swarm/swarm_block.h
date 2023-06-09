/*
 * swarm_piecewrapper.h
 *
 *  Created on: Jun 7, 2023
 *      Author: kiraly
 */

#ifndef SWARM_SWARM_BLOCK_H_
#define SWARM_SWARM_BLOCK_H_

#include <stddef.h>
#include <stdint.h>

typedef struct
{
	size_t thePieceIndex;
	size_t theOffsetWithinPiece;
	size_t theDataLength;
	uint8_t* theData;
} swarm_block_t;

swarm_block_t* swarm_block_constructBlockWrapper(size_t __thePieceIndex,size_t __theOffsetWithinPiece,size_t __theDataLength, void* __theData);
void swarm_block_destroyBlockDataAndWrapper(swarm_block_t* __theBlockWrapper);
void swarm_block_destroyBlockWrapper(swarm_block_t* __theBlockWrapper);


#endif /* SWARM_SWARM_BLOCK_H_ */
