/*
 * tcpswarm_proto.c
 *
 *  Created on: May 29, 2023
 *      Author: kiraly
 */

#include "tcpswarm_proto.h"
#include "tcpswarm_comm.h"
#include "concurrent_queue.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

void* _tcpswarm_processingFunction(void* __rawGenericPacket)
{

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
