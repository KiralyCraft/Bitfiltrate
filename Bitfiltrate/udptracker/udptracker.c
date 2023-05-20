/*
 * udptracker.c
 *
 *  Created on: May 9, 2023
 *      Author: kiraly
 */
#include "udptracker.h"
#include "udptracker_comm.h"
#include "udptracker_proto.h"

#include "../network/conpool.h"
#include "concurrent_queue.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include <dlinkedlist.h>

udptrack_t* udptracker_connect(const char* __givenTrackerURL,uint32_t __givenTrackerPort)
{
	int _socketDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (_socketDescriptor < 0)
	{
		//TODO error
	}

	//==== INVESTIGATING UDP TRACKER

	struct addrinfo* _urlAddress;

	int _lookupResult = getaddrinfo(__givenTrackerURL, NULL, NULL, &_urlAddress);
	if (_lookupResult != 0)
	{
		//TODO error
	}

	struct in_addr* _foundAddress = malloc(sizeof(struct in_addr*));
	for (struct addrinfo* rp = _urlAddress; rp != NULL; rp = rp->ai_next)
	{
		struct in_addr _iteratedAddress = ((struct sockaddr_in*)(rp->ai_addr))->sin_addr;

		if (memcmp(_foundAddress,&_iteratedAddress,sizeof(struct in_addr)) != 0)
		{
			memcpy(_foundAddress,&_iteratedAddress,sizeof(struct in_addr));
		}
	}
    freeaddrinfo(_urlAddress);

    //==== INITIALIZING UDP TRACKER TYPE
    udptrack_t* _theTrackerInfo = malloc(sizeof(udptrack_t));
    _theTrackerInfo->trackerStatus = UDPTRACK_CREATED;

    _theTrackerInfo->trackerSocket = _socketDescriptor;
    _theTrackerInfo->trackerIP = _foundAddress->s_addr;
    _theTrackerInfo->trackerPort = __givenTrackerPort;

    for (uint8_t _peerIDInitializer = 0; _peerIDInitializer < 20; _peerIDInitializer++)
    {
    	_theTrackerInfo->peerID[_peerIDInitializer] = rand() & 0xFF;
    }

    _theTrackerInfo->downloaded = 0;
    _theTrackerInfo->uploaded = 0;
    _theTrackerInfo->left = 0;

    free(_foundAddress);

    return _theTrackerInfo;
}

uint8_t udptracker_initialize(udptrack_t* __trackerData,conpool_t* __theConnectionPool)
{
	//==== CREATE THE CONNECTION POOL ====
	conc_queue* _outgoingQueue = conpool_createConnection(__theConnectionPool,__trackerData->trackerSocket,udptracker_comm_outgoingFunction,udptracker_comm_incomingFunction,udptracker_comm_processingFunction);
	__trackerData->trackerOutgoingPacketQueue = _outgoingQueue;
	__trackerData->trackerStatus = UDPTRACK_INITIALIZING;

	//==== PREPARE FOR CONVERSATION ===
	__trackerData->trackerConversations = dlinkedlist_createList();

	udptracker_conversation_t* _newConversation = malloc(sizeof(udptracker_conversation_t));
	_newConversation -> conversationType = UDP_TRACKER_PACKET_CONNECT;
	_newConversation -> converstationStatus = UDP_TRACKER_CONV_PENDING;
	_newConversation -> outgoingRequest = udptracker_proto_requestConnectPacket();
	_newConversation -> incomingRequest = NULL;

	uint8_t _conversationInsertionResult = dlinkedlist_insertElement(_newConversation,__trackerData->trackerConversations);

	if (_conversationInsertionResult == 0)
	{
		//TODO error
	}

	//==== SENDING PACKETS ====
	conc_queue_push(_outgoingQueue, _newConversation -> outgoingRequest);
	_newConversation -> converstationStatus = UDP_TRACKER_CONV_STARTED;
	return 0;
}

