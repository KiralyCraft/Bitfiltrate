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

#include "dlinkedlist.h"

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

uint8_t _udptracker_processResponseAction(udptrack_t* __tracketData,udptrack_packet_reply_t* __packetReply);

uint8_t udptracker_initialize(udptrack_t* __trackerData,conpool_t* __theConnectionPool)
{
	pthread_mutex_lock(&__trackerData->lockingMutex);
	//==== CREATE THE CONNECTION POOL ====
	conc_queue_t* _outgoingQueue = conpool_createConnection(__theConnectionPool,__trackerData->trackerNetworkConfiguration,udptracker_comm_outgoingFunction,udptracker_comm_incomingFunction,udptracker_proto_processRawGenericPacket,__trackerData,_udptracker_processResponseAction);
	__trackerData->trackerOutgoingPacketQueue = _outgoingQueue;
	__trackerData->trackerStatus = UDPTRACK_INITIALIZING;

	//==== PREPARE FOR CONVERSATION ===
	__trackerData->trackerConversations = dlinkedlist_createList();

	int32_t _newTransactionID = udptracker_proto_generateTransactionID();

	udptrack_conversation_t* _newConversation = malloc(sizeof(udptrack_conversation_t));
	_newConversation -> conversationType = UDP_TRACKER_PACKET_CONNECT;
	_newConversation -> converstationStatus = UDP_TRACKER_CONV_PENDING;
	_newConversation -> conversationID = _newTransactionID;
	_newConversation -> externalIdentifier = 0; //The connect packet shouldn't have an external identifier, since it is used across all torrents.

	udptrack_packet_t* _outgoingRequestPacket = udptracker_proto_requestConnectPacket(_newTransactionID);

	uint8_t _conversationInsertionResult = dlinkedlist_insertElement(_newConversation,__trackerData->trackerConversations);

	if (_conversationInsertionResult == 0)
	{
		//TODO error
	}

	//==== SENDING PACKETS ====
	conc_queue_push(_outgoingQueue, _outgoingRequestPacket);
	_newConversation -> converstationStatus = UDP_TRACKER_CONV_STARTED;

//	pthread_cond_broadcast (&__trackerData->updateCondvar); //Why is this needed? There's no reply yet?
	pthread_mutex_unlock(&__trackerData->lockingMutex);
	return 0;
}

/*
 * Helper function that compares a given key, supposedly an ID, with the ID of a conversation provided as an iterated element.
 *
 * This function returns 1 if a match is found, otherwise 0.
 */
uint8_t _udptracker_conversationIDComparator(void* __searchedKey, void* __iteratedElement)
{
	int32_t* _givenID = __searchedKey;
	udptrack_conversation_t* _theConversation = __iteratedElement;

	if (*_givenID == _theConversation->conversationID)
	{
		return 1;
	}
	return 0;

}

/*
 * This helper function processes the actual data received from the tracker, and should update the status accordingly.
 *
 * For packets that are destined to be used by something else (such as a watchdog), this method should not empty the packets, and neither the conversations.
 * For other packets and their subsequent conversations, the conversations should be cleared and emptied.
 *
 * This function is ALWAYS CALLED ASYNCHRONOUSLY.
 */
uint8_t _udptracker_processResponseAction(udptrack_t* __trackerData,udptrack_packet_reply_t* __packetReply)
{
	uint8_t _shouldClear = 1;

	int32_t _transactionID = __packetReply -> packetTransactionID;

	pthread_mutex_lock(&__trackerData->lockingMutex);

	//=== HANDLING THE PACKET DATA ===
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
		udptrack_packet_reply_data_announce_t* _packetReplyAnnounce = __packetReply -> packetData;
		_shouldClear = 0;
		printf("Tracker announce received\n");
	}
	else if (__packetReply -> packetType == UDP_TRACKER_PACKET_SCRAPE)
	{
		udptrack_packet_reply_data_scrape_t* _packetReplyScrape = __packetReply -> packetData;
		_shouldClear = 0;
		printf("Scrape received!");
	}
	else if (__packetReply -> packetType == UDP_TRACKER_PACKET_ERROR)
	{
		udptrack_packet_reply_data_error_t* _packetReplyError = __packetReply -> packetData;
		printf("Error received\n");
		//TODO handle the error packet, log it perhaps?
	}

	//=== UPDATING THE CONVERSATION STATUS ===
	udptrack_conversation_t* _currentConversation = dlinkedlist_getCustomElement(&_transactionID,_udptracker_conversationIDComparator,__trackerData->trackerConversations);
	_currentConversation->supplementalResponseData = __packetReply; //Set the packet reply inside the conversation data.
	if (_currentConversation == NULL)
	{
		printf("TODO: No es bueno, convo %d not found when process!!\n",_transactionID);
	}
	else
	{
		_currentConversation -> converstationStatus = UDP_TRACKER_CONV_FINISHED;

		if (_shouldClear == 1)
		{
			uint8_t _clearConversationSuccess = dlinkedlist_deleteElement(_currentConversation,__trackerData->trackerConversations);
			if (_clearConversationSuccess == 1)
			{
				free(_currentConversation); //Also free the conversation itself, it it has been successfully removed from the conversation list.
			}
			else
			{
				//TODO handle error
			}
		}

//		return true sau false, pentru anumite nu da clear la conversatii (si nici la pachete), dar pentru altele da.
	}

	pthread_cond_broadcast (&__trackerData->updateCondvar);
	pthread_mutex_unlock(&__trackerData->lockingMutex);

	return _shouldClear;
}

/*
 * This method is ASYNCRHONOUS!
 */
uint8_t udptracker_beginConversation(void* __requestPacket, udptrack_packet_type_t __conversationType, int32_t __transactionID ,int32_t __conversationExternalIdentifier, udptrack_t* __trackerData, uint8_t __shouldLock)
{
	udptrack_conversation_t* _newConversation = malloc(sizeof(udptrack_conversation_t));
	_newConversation -> conversationType = __conversationType;
	_newConversation -> converstationStatus = UDP_TRACKER_CONV_PENDING;
	_newConversation -> conversationID = __transactionID;
	_newConversation -> externalIdentifier = __conversationExternalIdentifier;

	if (__shouldLock)
	{
		pthread_mutex_lock(&__trackerData->lockingMutex);
	}
	uint8_t _conversationInsertionResult = dlinkedlist_insertElement(_newConversation,__trackerData->trackerConversations);

	if (_conversationInsertionResult == 0)
	{
		//TODO error
	}

	conc_queue_push(__trackerData->trackerOutgoingPacketQueue, __requestPacket);
	_newConversation -> converstationStatus = UDP_TRACKER_CONV_STARTED;

	if (__shouldLock)
	{
		pthread_mutex_unlock(&__trackerData->lockingMutex);
	}

	return 1;
}




