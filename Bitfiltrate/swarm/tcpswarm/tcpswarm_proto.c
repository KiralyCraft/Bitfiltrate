/*
 * tcpswarm_proto.c
 *
 *  Created on: May 29, 2023
 *      Author: kiraly
 */

#include "tcpswarm_proto.h"
#include "tcpswarm_comm.h"
#include "concurrent_queue.h"
#include "dlinkedlist.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "../swarm_block.h"

/*
 * This is a helper function that gets called by the processing function. It's purpose is to serve
 * as an encapsulated way of processing data which automatically gets cleared afterwards, such as the packaged packet.
 *
 * Packets processed here are destroyed afterwards.
 *
 * This function is always called ASYNCHRONOUSLY, just like the wrapper.
 */
void _tcpswarm_actualProcessingFunction(tcpswarm_packet_t* __packagedPacket,tcppeer_t* __thePeer,void* __optionalArguments)
{
//	aici puneai o regula ca daca length e deja 0 sa nu check, pentru ca nu are date keep-alive

	//=== SPECIAL CHECKS ===
	if (__packagedPacket->packetSize == 0)
	{
		return; //Do not even attempt to process messages with 0 length (likely keep-alive)
	}

	//=== PROCESSING THE PACKET ===
	uint8_t _packetTypeID = __packagedPacket->packetData[0];
	uint8_t* _packetDataOffset =  __packagedPacket->packetData+1;

	//=== MARKING PACKET AS SEEN ===
	uint8_t _packetSeenMask = 1 << _packetTypeID;
	__thePeer->packetsReceivedBitfield |= _packetSeenMask;
	//=== HANDLING PACKET ===
	if (_packetTypeID == 0) //Choked
	{
		__thePeer->peerChoking = 1; //TODO change these things to functions inside the peer definition, and call functions to set them
	}
	else if (_packetTypeID == 1) //Unchoked
	{
		__thePeer->peerChoking = 0;
	}
	else if (_packetTypeID == 2) //Interested
	{
		__thePeer->peerInterested = 1;
	}
	else if (_packetTypeID == 3) //Not Interested
	{
		__thePeer->peerInterested = 0;
	}
	else if (_packetTypeID == 4) //Have
	{
		uint32_t _pieceIndex = be32toh(((uint32_t*)_packetDataOffset)[0]);
		uint8_t _setPieceResult = tcppeer_setPiece(__thePeer,_pieceIndex,1);
		if (_setPieceResult == 0)
		{
			; //TODO handle this
		}
		printf("Received HAVE\n");
	}
	else if (_packetTypeID == 5) //Bitfield
	{
		printf("Recevied BITFIELD\n");
		size_t _receivedBitfieldSize = (__packagedPacket->packetSize - 1)/(sizeof(uint8_t));

		for (size_t _bitfieldByteIterator = 0; _bitfieldByteIterator < _receivedBitfieldSize; _bitfieldByteIterator++)
		{
			uint8_t _insertionResult = tcppeer_setBitfieldByte(__thePeer,_bitfieldByteIterator,_packetDataOffset[_bitfieldByteIterator]);
			if (_insertionResult == 0)
			{
				printf("Failed to add bitfield\n");
				; //TODO handle error inserting byte of bitfield
			}
		}
	}
	else if (_packetTypeID == 6) //Request - Not implemented
	{
		; //TODO tell them politely yet firmly to go themselves, we never answer requests
	}
	else if (_packetTypeID == 7) //Piece - Actual incoming data
	{
//		swarm_block_t* _theConstructedPiece = malloc(sizeof(swarm_block_t));
//		_theConstructedPiece->thePieceIndex = be32toh(((uint32_t*)_packetDataOffset)[0]);
//		_theConstructedPiece->theOffsetWithinPiece = be32toh(((uint32_t*)_packetDataOffset)[1]);
//		_theConstructedPiece->theDataLength = __packagedPacket->packetSize - sizeof(uint32_t)*2 - 1;
//
//		_theConstructedPiece->theData = malloc(_theConstructedPiece->theDataLength);
//		memcpy(_theConstructedPiece->theData, _packetDataOffset + sizeof(uint32_t)*2, _theConstructedPiece->theDataLength);

		size_t _thePieceIndex = be32toh(((uint32_t*)_packetDataOffset)[0]);
		size_t _theOffsetWithinPiece = be32toh(((uint32_t*)_packetDataOffset)[1]);
		size_t _theDataLength = __packagedPacket->packetSize - sizeof(uint32_t)*2 - 1;
		uint8_t* _blockDataOffset = _packetDataOffset + sizeof(uint32_t)*2;

		swarm_block_t* _theConstructedPiece = swarm_block_constructBlockWrapper(_thePieceIndex,_theOffsetWithinPiece,_theDataLength,_blockDataOffset);

		conc_queue_push(__thePeer->peerIncomingBlockData,_theConstructedPiece);
	}
	else if (_packetTypeID == 8) //Cancel - Not implemnted
	{
		; //TODO never needed, we never seed
	}
	else if (_packetTypeID == 9) //DHT Port - Not implemented
	{
		; //TODO what the hell is this?
	}
}

/*
 * This function is always called ASYNCHRONOUSLY, independent of everything else.
 */
void* _tcpswarm_processingFunction(void* __dataBundle)
{
	//=== SPLITTING THE DATA BUNDLE ===
	void** _splitDataBundle = __dataBundle;

	void* _rawGenericPacket = _splitDataBundle[0];
	void* _executionContext = _splitDataBundle[1];
	void* _optionalArguments = _splitDataBundle[2]; //this is the postprocessing function

	free(__dataBundle);

	//=== PROCESSING THE DATA BUNDLE ===
	if (_rawGenericPacket == NULL)
	{
		//TODO handle connection death here, we should really kill it.
//		printf("Received NULL when processing\n");
	}
	else
	{
		tcpswarm_packet_t* _packagedPacket = _rawGenericPacket;
		tcppeer_t* _thePeer = _executionContext;

		_tcpswarm_actualProcessingFunction(_packagedPacket,_thePeer,_optionalArguments);

		free(_packagedPacket->packetData);
		free(_packagedPacket);
	}

	return NULL;
}

/*
 * This method is NOT THREAD SAFE.
 */
uint8_t _tcpswarm_peerQueueOutgoingPacket(void* __thePacket,void* __thePeerData)
{
	tcppeer_t* _theTCPPeerData = __thePeerData;
	conc_queue_push(_theTCPPeerData->peerOutgoingPacketQueue,__thePacket);
	return 1;
}

void* _tcpswarm_generatePacket(swarm_message_e __theMessageType, void* __optionalData)
{
	tcpswarm_packet_t* _createdPacket = malloc(sizeof(tcpswarm_packet_t));

	if (__theMessageType == SWARM_MESSAGE_HANDSHAKE)
	{
		uint8_t* _torrentHashID = __optionalData;

		//=== CREATING THE OUTGOING PACKET ===
		_createdPacket->packetSize = sizeof(uint8_t)*68;
		_createdPacket->packetData = malloc(_createdPacket->packetSize);

		//=== FILLING PACKET DATA ===
		uint8_t* _currentPacketOffset = _createdPacket->packetData;

		//===> PSTRLEN
		((uint8_t*)_currentPacketOffset)[0] = 19;
		_currentPacketOffset += 1;
		//===> PSTR
		memcpy(_currentPacketOffset,"BitTorrent protocol",19);
		_currentPacketOffset += 19;
		//===> RESERVED
		_currentPacketOffset[0] = 0;
		_currentPacketOffset[1] = 0;
		_currentPacketOffset[2] = 0;
		_currentPacketOffset[3] = 0;
		_currentPacketOffset[4] = 0;
		_currentPacketOffset[5] = 0;
		_currentPacketOffset[6] = 0;
		_currentPacketOffset[7] = 0;
		_currentPacketOffset += 8;
		//===> INFO_HASH
		memcpy(_currentPacketOffset,_torrentHashID,20);
		_currentPacketOffset += 20;
		//===> PEER_ID
		memcpy(_currentPacketOffset,"-qB4450-dzjx7S4IOm!K",20);
	}
	else if (__theMessageType == SWARM_MESSAGE_CHOKE)
	{
		_createdPacket->packetSize = sizeof(uint32_t)+sizeof(uint8_t);
		_createdPacket->packetData = malloc(_createdPacket->packetSize);

		uint8_t* _currentPacketOffset = _createdPacket->packetData;
		//=== FILLING THE LENGTH ===
		((uint32_t*)_currentPacketOffset)[0] = htobe32(1);
		_currentPacketOffset += sizeof(uint32_t);
		//=== FILLING THE ID ===
		((uint8_t*)_currentPacketOffset)[0] = 0;
	}
	else if (__theMessageType == SWARM_MESSAGE_UNCHOKE)
	{
		_createdPacket->packetSize = sizeof(uint32_t)+sizeof(uint8_t);
		_createdPacket->packetData = malloc(_createdPacket->packetSize);

		uint8_t* _currentPacketOffset = _createdPacket->packetData;
		//=== FILLING THE LENGTH ===
		((uint32_t*)_currentPacketOffset)[0] = htobe32(1);
		_currentPacketOffset += sizeof(uint32_t);
		//=== FILLING THE ID ===
		((uint8_t*)_currentPacketOffset)[0] = 1;
	}
	else if (__theMessageType == SWARM_MESSAGE_INTERESTED)
	{
		_createdPacket->packetSize = sizeof(uint32_t)+sizeof(uint8_t);
		_createdPacket->packetData = malloc(_createdPacket->packetSize);

		uint8_t* _currentPacketOffset = _createdPacket->packetData;
		//=== FILLING THE LENGTH ===
		((uint32_t*)_currentPacketOffset)[0] = htobe32(1);
		_currentPacketOffset += sizeof(uint32_t);
		//=== FILLING THE ID ===
		((uint8_t*)_currentPacketOffset)[0] = 2;
	}
	else if (__theMessageType == SWARM_MESSAGE_NOT_INTERESTED)
	{
		_createdPacket->packetSize = sizeof(uint32_t)+sizeof(uint8_t);
		_createdPacket->packetData = malloc(_createdPacket->packetSize);

		uint8_t* _currentPacketOffset = _createdPacket->packetData;
		//=== FILLING THE LENGTH ===
		((uint32_t*)_currentPacketOffset)[0] = htobe32(1);
		_currentPacketOffset += sizeof(uint32_t);
		//=== FILLING THE ID ===
		((uint8_t*)_currentPacketOffset)[0] = 3;
	}
	else if (__theMessageType == SWARM_MESSAGE_REQUEST)
	{
		size_t* _informationBundle = __optionalData;
		_createdPacket->packetSize = sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint32_t)*3;
		_createdPacket->packetData = malloc(_createdPacket->packetSize);

		uint8_t* _currentPacketOffset = _createdPacket->packetData;
		//=== FILLING THE LENGTH ===
		((uint32_t*)_currentPacketOffset)[0] = htobe32(13);
		_currentPacketOffset += sizeof(uint32_t);
		//=== FILLING THE ID ===
		((uint8_t*)_currentPacketOffset)[0] = 6;
		_currentPacketOffset += sizeof(uint8_t);
		//=== FILLING THE DATA ===
		((uint32_t*)_currentPacketOffset)[0] = htobe32(_informationBundle[0]);
		((uint32_t*)_currentPacketOffset)[1] = htobe32(_informationBundle[1]);
		((uint32_t*)_currentPacketOffset)[2] = htobe32(_informationBundle[2]);

	}
	return _createdPacket;
}
