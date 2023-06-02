/*
 * tcpswarm_filters.h
 *
 *  Created on: Jun 2, 2023
 *      Author: kiraly
 */

#ifndef SWARM_TCPSWARM_TCPSWARM_FILTERS_H_
#define SWARM_TCPSWARM_TCPSWARM_FILTERS_H_

#include "../swarm_filters.h"
#include "dlinkedlist.h"

#include <stdint.h>

/*
 * These functions filter peers into the given bucket according to the filter criteria.
 *
 * This function returns 0 if anything went wrong, or 1 othrwise.
 *
 * This function assumes it has exclusive access over the peer list.
 */


/*
 * This function assumes the filterCriteria is a pointer to a locally-defined variable of the type
 * corresponding to the network status of a peer.
 */
uint8_t tcpswarm_filters_filterByNetworkStatus(dlinkedlist_t* __thePeerList,swarm_filters_peerdata_t* __theFilteredPeers, void* __filterCriteria);
uint8_t tcpswarm_filters_filterByIncomingPacketTime(dlinkedlist_t* __thePeerList,swarm_filters_peerdata_t* __theFilteredPeers, void* __filterCriteria);
uint8_t tcpswarm_filters_filterByOutgoingPacketTime(dlinkedlist_t* __thePeerList,swarm_filters_peerdata_t* __theFilteredPeers, void* __filterCriteria);
uint8_t tcpswarm_filters_filterByNonzeroIncomingPacketCount(dlinkedlist_t* __thePeerList,swarm_filters_peerdata_t* __theFilteredPeers, void* __filterCriteria);

#endif /* SWARM_TCPSWARM_TCPSWARM_FILTERS_H_ */
