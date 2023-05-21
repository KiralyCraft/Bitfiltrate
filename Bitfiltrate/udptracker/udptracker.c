/*
 * udptracker.c
 *
 *  Created on: May 9, 2023
 *      Author: kiraly
 */
#include "udptracker.h"

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

udptrack_t* udptracker_create(const char* __givenTrackerURL,uint32_t __givenTrackerPort)
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

    pthread_mutex_init(&_theTrackerInfo->lockingMutex,NULL);
    pthread_cond_init(&_theTrackerInfo->updateCondvar,NULL);

    _theTrackerInfo->trackerNetworkConfiguration = malloc(sizeof(udptrack_networkconfig));
    _theTrackerInfo->trackerNetworkConfiguration->trackerSocket = _socketDescriptor;
    _theTrackerInfo->trackerNetworkConfiguration->trackerIP = _foundAddress->s_addr;
    _theTrackerInfo->trackerNetworkConfiguration->trackerPort = __givenTrackerPort;

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

void _udptracker_processResponseAction(udptrack_t* __tracketData,udptrack_packet_reply_t* __packetReply);

uint8_t udptracker_initialize(udptrack_t* __trackerData,conpool_t* __theConnectionPool)
{
	pthread_mutex_lock(&__trackerData->lockingMutex);
	//==== CREATE THE CONNECTION POOL ====
	conc_queue* _outgoingQueue = conpool_createConnection(__theConnectionPool,__trackerData->trackerNetworkConfiguration,udptracker_comm_outgoingFunction,udptracker_comm_incomingFunction,udptracker_proto_processRawGenericPacket,__trackerData,_udptracker_processResponseAction);
	__trackerData->trackerOutgoingPacketQueue = _outgoingQueue;
	__trackerData->trackerStatus = UDPTRACK_INITIALIZING;

	//==== PREPARE FOR CONVERSATION ===
	__trackerData->trackerConversations = dlinkedlist_createList();

	udptrack_conversation_t* _newConversation = malloc(sizeof(udptrack_conversation_t));
	_newConversation -> conversationType = UDP_TRACKER_PACKET_CONNECT;
	_newConversation -> converstationStatus = UDP_TRACKER_CONV_PENDING;
	_newConversation -> outgoingRequest = udptracker_proto_requestConnectPacket(); //TODO replace this with an actual structure for outgoing packets, not things borrowed from the comm, which we should not care about
	_newConversation -> incomingRequest = NULL;

	uint8_t _conversationInsertionResult = dlinkedlist_insertElement(_newConversation,__trackerData->trackerConversations);

	if (_conversationInsertionResult == 0)
	{
		//TODO error
	}

	//==== SENDING PACKETS ====
	conc_queue_push(_outgoingQueue, _newConversation -> outgoingRequest);
	_newConversation -> converstationStatus = UDP_TRACKER_CONV_STARTED;

	pthread_cond_signal (&__trackerData->updateCondvar);
	pthread_mutex_unlock(&__trackerData->lockingMutex);
	return 0;
}

/*
 * This helper function processes the actual data received from the tracker, and should update the status accordingly.
 *
 * This function is ALWAYS CALLED ASYNCHRONOUSLY.
 */
void _udptracker_processResponseAction(udptrack_t* __trackerData,udptrack_packet_reply_t* __packetReply)
{
	int32_t _transactionID = __packetReply -> packetTransactionID;

	pthread_mutex_lock(&__trackerData->lockingMutex);

	if (__packetReply -> packetType == UDP_TRACKER_PACKET_CONNECT)
	{
		//=== EXTRACTING PACKET DATA ===
		udptrack_packet_reply_data_connect_t* _packetReplyConnect = __packetReply -> packetData;
		__trackerData->connectionID = _packetReplyConnect->connectionID;

		//=== UPDATING TRACKER STATUS ===
		__trackerData->trackerStatus = UDPTRACK_INITIALIZED;
	}
	else if (__packetReply -> packetType == UDP_TRACKER_PACKET_ANNOUNCE)
	{

	}
	else if (__packetReply -> packetType == UDP_TRACKER_PACKET_SCRAPE)
	{

	}
	else if (__packetReply -> packetType == UDP_TRACKER_PACKET_ERROR)
	{
		udptrack_packet_reply_data_error_t* _packetReplyError = __packetReply -> packetData;
		//TODO handle the error packet, log it perhaps?
	}

	free(__packetReply->packetData);
	free(__packetReply);

	pthread_cond_signal (&__trackerData->updateCondvar);
	pthread_mutex_unlock(&__trackerData->lockingMutex);
	//TODO end conversation here, and clear & free the incoming & outgoing packets. Should probably remove the conversation from the dlinkedlist as well, but it needs to be thread safe
}



