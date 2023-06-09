/*
 * swarm_block.c
 *
 *  Created on: Jun 7, 2023
 *      Author: kiraly
 */

#include "swarm_block.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

swarm_block_t* swarm_block_constructBlockWrapper(size_t __thePieceIndex,size_t __theOffsetWithinPiece,size_t __theDataLength, void* __theData)
{
	swarm_block_t* _theConstructedPiece = malloc(sizeof(swarm_block_t));
	_theConstructedPiece->thePieceIndex = __thePieceIndex;
	_theConstructedPiece->theOffsetWithinPiece = __theOffsetWithinPiece;
	_theConstructedPiece->theDataLength = __theDataLength;
	_theConstructedPiece->theData = malloc(_theConstructedPiece->theDataLength);
	memcpy(_theConstructedPiece->theData, __theData, _theConstructedPiece->theDataLength);

	return _theConstructedPiece;
}

void swarm_block_destroyBlockDataAndWrapper(swarm_block_t* __theBlockWrapper)
{
	free(__theBlockWrapper->theData);
	free(__theBlockWrapper);
}
void swarm_block_destroyBlockWrapper(swarm_block_t* __theBlockWrapper)
{
	free(__theBlockWrapper);
}

