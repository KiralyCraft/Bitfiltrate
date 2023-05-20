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
	//==== CONNECTIVITY ====
	int trackerSocket;
	in_addr_t trackerIP;
	uint32_t trackerPort;
	conc_queue* trackerOutgoingPacketQueue;
	dlinkedlist_t* trackerConversations;
	//==== SPECIFICS ====
	/*
	 * Sent by the tracker upon connection
	 */
	uint64_t connectionID;
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
} udptrack_t;

/*
 * Creates a new connection to the specified tracker host, at the specified port.
 *
 * It returns a udptrack_t element, which is configured but not yet initialized. The
 * server has not been talked to yet, but it has been identified and prepared for.
 *
 * The connection must be initialized in order to become useful.
 */
udptrack_t* udptracker_connect(const char* __host,uint32_t __port);

/*
 * Initializes this tracker, the support for conversations, and everything else.
 * Returns a success value depending on the situation, returns 0 if something failed, or 1 otherwise.
 */
uint8_t udptracker_initialize(udptrack_t* __trackerData,conpool_t* __theConnectionPool);

#endif /* UDPTRACKER_UDPTRACKER_H_ */
