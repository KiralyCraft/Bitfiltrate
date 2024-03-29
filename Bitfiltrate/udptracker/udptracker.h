/*
 * udptracker.h
 *
 *  Created on: May 9, 2023
 *      Author: kiraly
 */

#ifndef UDPTRACKER_UDPTRACKER_H_
#define UDPTRACKER_UDPTRACKER_H_

#include <stdint.h>
#include <netinet/in.h>
#include "concurrent_queue.h"
#include "../network/conpool.h"

#include "udptracker_comm.h"
#include "udptracker_proto.h"

#include "dlinkedlist.h"

typedef enum
{
	UDPTRACK_CREATED,
	UDPTRACK_INITIALIZING,
	UDPTRACK_INITIALIZED,
	UDPTRACK_ERROR
} udptrack_configstatus_t;

typedef struct
{
	int trackerSocket;
	in_addr_t trackerIP;
	uint32_t trackerPort;
} udptrack_networkconfig;

typedef struct
{
	//==== CONNECTIVITY ====
	udptrack_networkconfig* trackerNetworkConfiguration;
	conc_queue_t* trackerOutgoingPacketQueue;
	dlinkedlist_t* trackerConversations;
	//==== SPECIFICS ====
	/*
	 * Sent by the tracker upon connection
	 */
	int64_t connectionID;
	/*
	 * Randomly generated peer id
	 */
	uint8_t peerID[20];
	//==== STATUS ====
	/*
	 * Statistics regarding the number of bytes since "started".
	 */
	uint64_t downloaded;
	uint64_t uploaded;
	uint64_t left;

	/*
	 * Keep track of this tracker's initialization status.
	 */
	udptrack_configstatus_t trackerStatus;

	//=== WATCHDOG STUFF ===
	pthread_mutex_t lockingMutex;
	pthread_cond_t updateCondvar;



} udptrack_t;

/*
 * The state of a conversation with an UDP tracker. It can be:
 *	UDP_TRACKER_CONV_PENDING - The conversation is set to start at some point
 *	UDP_TRACKER_CONV_STARTED - The conversation has been started, and is waiting for a response
 *	UDP_TRACKER_CONV_FINISHED - The conversation has been replied to, and is considered finished
 *	UDP_TRACKER_CONV_TIMEOUT - The conversation timed out while waiting for a reply
 */
typedef enum
{
	UDP_TRACKER_CONV_PENDING,
	UDP_TRACKER_CONV_STARTED,
	UDP_TRACKER_CONV_FINISHED,
	UDP_TRACKER_CONV_TIMEOUT
} udptrack_conversation_status_t;

/*
 * A data structure that keeps track of conversations with a tracker.
 */
typedef struct
{
	udptrack_conversation_status_t converstationStatus;
	udptrack_packet_type_t conversationType;
	/*
	 * This is just the transaction ID of the outgoing packet.
	 */
	int32_t conversationID;

	/*
	 * An optional external identifier. This is used by external factors
	 * to find this conversation based on this (not necessarily unique)
	 * identifier.
	 */
	int32_t externalIdentifier;

	/*
	 * Supplemental response data.
	 * Configured when the conversation receives a response.
	 */
	void* supplementalResponseData;
} udptrack_conversation_t;

/*
 * Creates a new connection to the specified tracker host, at the specified port.
 *
 * It returns a udptrack_t element, which is configured but not yet initialized. The
 * server has not been talked to yet, but it has been identified and prepared for.
 *
 * The connection must be initialized in order to become useful.
 */
udptrack_t* udptracker_create(const char* __host,uint32_t __port);

/*
 * Initializes this tracker, the support for conversations, and everything else.
 * Returns a success value depending on the situation, returns 0 if something failed, or 1 otherwise.
 */
uint8_t udptracker_initialize(udptrack_t* __trackerData,conpool_t* __theConnectionPool);

/*
 * Begins a conversation of the given type, containing the given data, which also contains an identifier.
 *
 * The request data is a generic packet which is to be sent out by the tracker. The response can then be found based
 * on the EXTERNAL conversation identifier (different from the internal one).
 *
 * This method is likely to be called from external factors, therefore it's implementation should be thread safe.
 *
 * This method returns 0 if anything went wrong, or 1 otherwise.
 */
uint8_t udptracker_beginConversation(void* __requestPacket, udptrack_packet_type_t __conversationType,int32_t __transactionID, int32_t __conversationExternalIdentifier, udptrack_t* __trackerData, uint8_t __shouldLock);

#endif /* UDPTRACKER_UDPTRACKER_H_ */
