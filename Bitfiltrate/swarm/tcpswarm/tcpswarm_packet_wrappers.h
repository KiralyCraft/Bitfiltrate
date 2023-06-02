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

//typedef struct
//{
//	size_t thePieceIndex;
//	size_t thePieceOffset;
//	size_t theIntendedLength;
//} tcpswarm_proto_packet_request; //This should be used for outgoing purposes, but our implementation avoids it

#endif /* SWARM_TCPSWARM_TCPSWARM_PACKET_WRAPPERS_H_ */
