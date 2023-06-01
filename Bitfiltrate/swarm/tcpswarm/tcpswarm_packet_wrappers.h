/*
 * tcpswarm_packet_wrappers.h
 *
 *  Created on: Jun 1, 2023
 *      Author: kiraly
 */

#ifndef SWARM_TCPSWARM_TCPSWARM_PACKET_WRAPPERS_H_
#define SWARM_TCPSWARM_TCPSWARM_PACKET_WRAPPERS_H_

#include <stddef.h>
#include <stdint.h>

typedef struct
{
	size_t thePieceIndex;
	size_t theOffsetWithinPiece;
	size_t theDataLength;
	uint8_t* theData;
} tcpswarm_proto_packet_piece;

#endif /* SWARM_TCPSWARM_TCPSWARM_PACKET_WRAPPERS_H_ */
