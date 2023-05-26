/*
 * udptracker_proto.c
 *
 *  Created on: May 20, 2023
 *      Author: kiraly
 */

#include "udptracker_proto.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <endian.h>

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
	((int64_t*)_packetData)[0] = htobe64(0x41727101980);
	((int32_t*)_packetData)[2] = htobe32(0);
	((int32_t*)_packetData)[3] = htobe32(__transactionID);

	return _newPacket;

}

udptrack_packet_t* udptracker_proto_requestScrapePacket(int64_t __connectionID,uint8_t* __torrentHash, int32_t __transactionID)
{
	//==== SETTING UP THE PACKET ====
	udptrack_packet_t* _newPacket = malloc(sizeof(udptrack_packet_t));
	size_t _packetDataLength = sizeof(int64_t)+sizeof(int32_t)*2+sizeof(uint8_t)*20;
	uint8_t* _packetData = malloc(_packetDataLength);
	_newPacket -> packetData = _packetData;
	_newPacket -> packetSize = _packetDataLength;
	//==== SETTING THE DATA ====

	((int64_t*)_packetData)[0] = htobe64(__connectionID);
	((int32_t*)_packetData)[2] = htobe32(2);
	((int32_t*)_packetData)[3] = htobe32(__transactionID);

	uint8_t* _packetDataHashOffset = _packetData + sizeof(int32_t)*4;
	memcpy(_packetDataHashOffset,__torrentHash, sizeof(uint8_t)*20);

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
	int32_t _packetAction = be32toh(((int32_t*)_packetData)[0]);
	int32_t _transactionID = be32toh(((int32_t*)_packetData)[1]);

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

	udptracker_proto_destroyPacket(_receivedActualPacket); //Destroy the actual packet data, because it has been processed into the nicer form
	//=== SERVING THE BUILT PACKET ===

	void (*_upperProcessingFunction)(void*,void*) = _optionalArguments;
	_upperProcessingFunction(_executionContext,_packetReply);

	//=== CLEANING THE NICELY WRAPPED PACKET ===
	udptracker_proto_destroyReplyPacket(_packetReply); //Destroy the nicely wrapped packet too, because it has been processed
	return NULL;
}

/*
 * Warning - This function is always called asynchronously!
 */
void* _udptracker_proto_processConnectPacket(udptrack_packet_t* __receivedPacketData)
{
	uint8_t* _receivedPacketDataOffset = __receivedPacketData->packetData + sizeof(int32_t)*2;

	udptrack_packet_reply_data_connect_t* _replyConnect = malloc(sizeof(udptrack_packet_reply_data_connect_t));

	_replyConnect -> connectionID = be64toh(((int64_t*)_receivedPacketDataOffset)[0]);
	return _replyConnect;
}

/*
 * Warning - This function is always called asynchronously!
 */
void* _udptracker_proto_processAnnouncePacket(udptrack_packet_t* __receivedPacketData)
{
	uint8_t* _receivedPacketDataOffset = __receivedPacketData->packetData + sizeof(int32_t)*2;

	udptrack_packet_reply_data_announce_t* _replyAnnounce = malloc(sizeof(udptrack_packet_reply_data_announce_t));
	//=== PROCESSING STATIC ELEMENTS ===
	_replyAnnounce -> reannounceInterval = be32toh(((int32_t*)_receivedPacketDataOffset)[0]);
	_replyAnnounce -> leechCount = be32toh(((int32_t*)_receivedPacketDataOffset)[1]);
	_replyAnnounce -> seedCount = be32toh(((int32_t*)_receivedPacketDataOffset)[2]);

	//=== PROCESSING PEER LIST ===

	size_t _peerPacketSize = sizeof(int32_t) + sizeof(uint16_t);
	uint32_t _expectedPeerCount = (__receivedPacketData->packetSize - (sizeof(int32_t)*2 + sizeof(int32_t)*3)) / _peerPacketSize;

	_replyAnnounce -> thePeerCount = _expectedPeerCount;
	_replyAnnounce -> thePeerList = malloc(sizeof(udptrack_packet_reply_data_announce_peer_t) * _expectedPeerCount);

	uint8_t* _receivedPacketPeerOffset = _receivedPacketDataOffset + sizeof(int32_t)*3;

	for (uint32_t _peerIndex = 0; _peerIndex < _expectedPeerCount; _peerIndex++)
	{
		_replyAnnounce -> thePeerList[_peerIndex].peerIP = be32toh(((int32_t*)(_receivedPacketPeerOffset + _peerIndex * _peerPacketSize))[0]);
		_replyAnnounce -> thePeerList[_peerIndex].peerPort = be32toh(((uint16_t*)(_receivedPacketPeerOffset + _peerIndex * _peerPacketSize + sizeof(int32_t)))[0]);
	}

	return _replyAnnounce;
}

/*
 * Warning - This function is always called asynchronously!
 */
void* _udptracker_proto_processScrapePacket(udptrack_packet_t* __receivedPacketData)
{
	uint8_t* _receivedPacketDataOffset = __receivedPacketData->packetData + sizeof(int32_t)*2;

	udptrack_packet_reply_data_scrape_t* _replyScrape = malloc(sizeof(udptrack_packet_reply_data_scrape_t));

	_replyScrape -> completeCount = be32toh(((int32_t*)_receivedPacketDataOffset)[0]);
	_replyScrape -> downloadCount = be32toh(((int32_t*)_receivedPacketDataOffset)[1]);
	_replyScrape -> incompleteCount = be32toh(((int32_t*)_receivedPacketDataOffset)[2]);

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

void udptracker_proto_destroyPacket(udptrack_packet_t* __thePacket)
{
	free(__thePacket->packetData);
	free(__thePacket);
}

void udptracker_proto_destroyReplyPacket(udptrack_packet_reply_t* __thePacket)
{
	udptrack_packet_type_t _replyPacketType = __thePacket->packetType;

	if (_replyPacketType == UDP_TRACKER_PACKET_CONNECT)
	{
		free(__thePacket);
	}
	else if (_replyPacketType == UDP_TRACKER_PACKET_ANNOUNCE)
	{
		udptrack_packet_reply_data_announce_t* _replyAnnounce = __thePacket->packetData;
		free(_replyAnnounce->thePeerList);
		free(_replyAnnounce);
	}
	else if (_replyPacketType == UDP_TRACKER_PACKET_SCRAPE)
	{
		free(__thePacket);
	}
	else if (_replyPacketType == UDP_TRACKER_PACKET_ERROR)
	{
		udptrack_packet_reply_data_error_t* _replyError = __thePacket->packetData;
		free(_replyError->errorMessage);
		free(_replyError);
	}
}

