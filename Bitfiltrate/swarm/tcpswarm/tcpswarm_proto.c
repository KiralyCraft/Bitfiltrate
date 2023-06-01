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

/*
 * This function is always called ASYNCHRONOUSLY, independent of everything else.
 */
void* _tcpswarm_processingFunction(void* __dataBundle)
{
	//=== SPLITTING THE DATA BUNDLE ===
	void** _splitDataBundle = __dataBundle;

	void* _rawGenericPacket = _splitDataBundle[0];
	void* _executionContext = _splitDataBundle[1];
	void* _optionalArguments = _splitDataBundle[2];

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
		//=== PROCESSING THE PACKET ===
		uint8_t _packetTypeID = _packagedPacket->packetData[0];

		printf("Got packet of type %d\n",_packetTypeID);
		if (_packetTypeID == 0)
		{
			_thePeer->peerChoking = 1; //TODO change these things to functions inside the peer definition, and call functions to set them
		}
		else if (_packetTypeID == 1)
		{
			_thePeer->peerChoking = 0;
		}
		else if (_packetTypeID == 2)
		{
			_thePeer->peerInterested = 1;
		}
		else if (_packetTypeID == 3)
		{
			_thePeer->peerInterested = 0;
		}
		else if (_packetTypeID == 4)
		{
			//TODO add info to bitfield
		}
		else if (_packetTypeID == 5)
		{
			size_t _receivedBitfieldSize = (_packagedPacket->packetSize - 1)/(sizeof(uint8_t));
			uint8_t* _bitfieldDataOffset = _packagedPacket->packetData + 1;

			for (size_t _bitfieldByteIterator = 0; _bitfieldByteIterator < _receivedBitfieldSize; _bitfieldByteIterator++)
			{
				uint8_t _insertionResult = tcppeer_setBitfieldByte(_thePeer,_bitfieldByteIterator,_bitfieldDataOffset[_bitfieldByteIterator]);
				if (_insertionResult == 0)
				{
					printf("Failed to add bitfield\n");
					; //TODO handle error inserting byte of bitfield
				}
			}
		}

//		receive bitfields from peers, and keep track of them. then, it may be up to the watchdog (possibly notify it) that
//				an update has been received. then, attempt to request data with various piece sizes, see what sticks.
//		beware that some clients may not send the whole bitfield, but rather send a few and then continue with "have" pieces

	}
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
		_createdPacket->packetData = malloc(sizeof(uint8_t)*68);
		_createdPacket->packetSize = sizeof(uint8_t)*68;

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
//		memcpy(_currentPacketOffset,"-AZ2060-",8);
		memcpy(_currentPacketOffset,"-qB4450-dzjx7S4IOm!K",20);
		//Leave this random, uninitialized :)
	}
	return _createdPacket;
}
