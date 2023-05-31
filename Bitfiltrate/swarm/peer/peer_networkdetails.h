/*
 * peer_networkdetails.h
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#ifndef SWARM_PEER_PEER_NETWORKDETAILS_H_
#define SWARM_PEER_PEER_NETWORKDETAILS_H_

#include <stdint.h>

typedef enum
{
	PEER_NEWBORN, //The status a new peer structure gets when it is first initialized
	PEER_PENDING, //The status of a new peer that is currently connecting
	PEER_INITIALIZED, //The status a peer is updated to after being submitted to a connection pool
	PEER_CONNECTED, //The status a peer gets after having done a successful handshake
	PEER_ERROR //The general error status
} peer_networkconfig_status_h;

/*
 * A structure defining the network status of a peer. This is originally unconnected, and therefore
 * the status of the socket is not guaranteed to be valid until the peer becomes connected.
 */
typedef struct
{
	int peerSocket;
	uint32_t peerIP;
	uint16_t peerPort;

	peer_networkconfig_status_h peerConnectionStatus;
} peer_networkconfig_h;

/*
 * Generates the network configuration for a typical peer, regardless of it's connection type (TCP or uTP/UDP).
 */
peer_networkconfig_h* peer_networkdetails_generatePeerDetails(uint32_t __peerIP,uint16_t __peerPort);

#endif /* SWARM_PEER_PEER_NETWORKDETAILS_H_ */
