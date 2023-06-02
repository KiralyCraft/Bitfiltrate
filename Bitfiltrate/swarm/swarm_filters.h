/*
 * swarm_filters.h
 *
 *  Created on: Jun 2, 2023
 *      Author: kiraly
 */

#ifndef SWARM_SWARM_FILTERS_H_
#define SWARM_SWARM_FILTERS_H_

#include "dlinkedlist.h"

#include <stddef.h>

typedef enum
{
	/*
	 * Used to set the filter into giving information related to the peer's network status.
	 */
	SWARM_PEERFILTER_NETWORKSTATUS,
	/*
	 * Used to set the filter into giving information related to the timing of packets received by the peer
	 */
	SWARM_PEERFILTER_PACKETTIME_INCOMING,
	/*
	 * Used to set the filter into giving information related to the timing of packets sent by the peer
	 */
	SWARM_PEERFILTER_PACKETTIME_OUTGOING,
	/*
	 * Used to filter peers who have received at least one instance of this packet.
	 */
	SWARM_PEERFILTER_PACKETCOUNT_INCOMING_NONZERO
} swarm_filters_peerdata_criteria_e;

typedef struct
{
	swarm_filters_peerdata_criteria_e peerFilterCriteria;
	void* peerFilterData;
} swarm_filters_peerdata_criteria_t;

/*
 * This data structure is what filter functions return, based on peers.
 */
typedef struct
{
	dlinkedlist_t* peerData;
} swarm_filters_peerdata_t;

/*
 * Creates, prepares and initializes a bucket for communicating filtered entities.
 */
swarm_filters_peerdata_t* swarm_filters_createPeerFilterBucket();
/*
 * Attempts to destroy a bucket containing filtered peers. In the event of failure, one
 * may try again to destroy this bucket.
 */
uint8_t swarm_filters_destroyPeerFilterBucket(swarm_filters_peerdata_t* __theFilteredBucket);

#endif /* SWARM_SWARM_FILTERS_H_ */
