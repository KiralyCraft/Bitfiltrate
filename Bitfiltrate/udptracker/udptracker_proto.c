/*
 * udptracker_proto.c
 *
 *  Created on: May 20, 2023
 *      Author: kiraly
 */

#include "udptracker_proto.h"
#include <stdlib.h>
#include <arpa/inet.h>

int32_t udptracker_proto_generateTransactionID()
{
	return rand(); //rofl
}

udptrack_packet_t* udptracker_proto_requestConnectPacket(int32_t __transactionID)
{
	//==== SETTING UP THE PACKET ====
	udptrack_packet_t* _newPacket = malloc(sizeof(udptrack_packet_t));
	size_t _packetDataLength = sizeof(int64_t)+sizeof(int32_t)*2;
	uint8_t* _packetData = malloc(_packetDataLength);
	_newPacket -> packetData = _packetData;
	_newPacket -> packetSize = _packetDataLength;
	//==== SETTING THE DATA ====

	/*
	 * Signature is 0x41727101980 - A 64 bit integer.
	 * It must be in network byte order (big endian), so split in two and big endian it
	 */
	((int32_t*)_packetData)[0] = htonl(0x0417);
	((int32_t*)_packetData)[1] = htonl(0x27101980);
	((int32_t*)_packetData)[2] = htonl(0);
	((int32_t*)_packetData)[3] = htonl(__transactionID);

	return _newPacket;

}

void* _udptracker_proto_processConnectPacket(udptrack_packet_t* __receivedPacketData);
void* _udptracker_proto_processAnnouncePacket(udptrack_packet_t* __receivedPacketData);
void* _udptracker_proto_processScrapePacket(udptrack_packet_t* __receivedPacketData);
void* _udptracker_proto_processErrorPacket(udptrack_packet_t* __receivedPacketData);

/*
 * Warning - This function is always called asynchronously!
 */
void* udptracker_proto_processRawGenericPacket(void* __dataBundle)
{
	//=== SPLITTING THE DATA BUNDLE ===
	void** _splitDataBundle = __dataBundle;

	void* _receivedPacket = _splitDataBundle[0];
	void* _executionContext = _splitDataBundle[1];
	void* _optionalArguments = _splitDataBundle[2];

	free(__dataBundle);

	//=== PROCESSING THE DATA BUNDLE ===
	udptrack_packet_t* _receivedActualPacket = _receivedPacket;

	uint8_t* _packetData = _receivedActualPacket -> packetData;
	int32_t _packetAction = ntohl(((int32_t*)_packetData)[0]);
	int32_t _transactionID = ntohl(((int32_t*)_packetData)[1]);

	udptrack_packet_reply_t* _packetReply = malloc(sizeof(udptrack_packet_reply_t));

	//=== BUILDING THE REPLY PACKET ===

	_packetReply -> packetTransactionID = _transactionID;
	if (_packetAction == 0) //Connect
	{
		_packetReply -> packetType = UDP_TRACKER_PACKET_CONNECT;
		_packetReply -> packetData = _udptracker_proto_processConnectPacket(_receivedActualPacket);
	}
	else if (_packetAction == 1) //Announce
	{
		_packetReply -> packetType = UDP_TRACKER_PACKET_ANNOUNCE;
		_packetReply -> packetData = _udptracker_proto_processAnnouncePacket(_receivedActualPacket);
	}
	else if (_packetAction == 2) //Scrape
	{
		_packetReply -> packetType = UDP_TRACKER_PACKET_SCRAPE;
		_packetReply -> packetData = _udptracker_proto_processScrapePacket(_receivedActualPacket);
	}
	else if (_packetAction == 3)
	{
		_packetReply -> packetType = UDP_TRACKER_PACKET_ERROR;
		_packetReply -> packetData = _udptracker_proto_processErrorPacket(_receivedActualPacket);
	}
	//=== SERVING THE BUILT PACKET ===

	void (*_upperProcessingFunction)(void*,void*) = _optionalArguments;
	_upperProcessingFunction(_executionContext,_packetReply);

	udptracker_proto_destroyPacket();
	return NULL;
}

/*
 * Warning - This function is always called asynchronously!
 */
void* _udptracker_proto_processConnectPacket(udptrack_packet_t* __receivedPacketData)
{
	uint8_t* _receivedPacketDataOffset = __receivedPacketData->packetData + sizeof(int32_t)*2;

	udptrack_packet_reply_data_connect_t* _replyConnect = malloc(sizeof(udptrack_packet_reply_data_connect_t));
	_replyConnect -> connectionID = ((int64_t*)_receivedPacketDataOffset)[0]; //The previous 64 bits are occupied by packet data
	return _replyConnect;
}

/*
 * Warning - This function is always called asynchronously!
 */
void* _udptracker_proto_processAnnouncePacket(udptrack_packet_t* __receivedPacketData)
{
	uint8_t* _receivedPacketDataOffset = __receivedPacketData->packetData + sizeof(int32_t)*2;

	udptrack_packet_reply_data_announce_t* _replyAnnounce = malloc(sizeof(udptrack_packet_reply_data_announce_t));

	_replyAnnounce -> reannounceInterval = ((int32_t*)_receivedPacketDataOffset)[0];
	_replyAnnounce -> leechCount = ((int32_t*)_receivedPacketDataOffset)[1];
	_replyAnnounce -> seedCount = ((int32_t*)_receivedPacketDataOffset)[2];

	printf("DEBUG: Announce left %d bytes\n",__receivedPacketData->packetSize);
	return _replyAnnounce;
}

/*
 * Warning - This function is always called asynchronously!
 */
void* _udptracker_proto_processScrapePacket(udptrack_packet_t* __receivedPacketData)
{
	uint8_t* _receivedPacketDataOffset = __receivedPacketData->packetData + sizeof(int32_t)*2;

	udptrack_packet_reply_data_scrape_t* _replyScrape = malloc(sizeof(udptrack_packet_reply_data_scrape_t));

	_replyScrape -> completeCount = ((int32_t*)_receivedPacketDataOffset)[0];
	_replyScrape -> downloadCount = ((int32_t*)_receivedPacketDataOffset)[1];
	_replyScrape -> incompleteCount = ((int32_t*)_receivedPacketDataOffset)[2];

	return _replyScrape;
}

/*
 * Warning - This function is always called asynchronously!
 */
void* _udptracker_proto_processErrorPacket(udptrack_packet_t* __receivedPacketData)
{
	//TODO parse this please :)
	return NULL;
}
