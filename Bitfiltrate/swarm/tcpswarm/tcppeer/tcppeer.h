/*
 * peer.h
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#ifndef SWARM_TCPPEER_H_
#define SWARM_TCPPEER_H_

#include <stdint.h>

#include <pthread.h>
#include "concurrent_queue.h"
#include "../../peer/peer_networkdetails.h"

typedef struct
{
	//==== CONNECTIVITY ====
	peer_networkconfig_h* peerNetworkConfig;
	conc_queue* peerOutgoingPacketQueue;
	//==== PEER-SPECIFIC ===

	uint8_t amChocking;
	uint8_t amInterested;
	uint8_t peerChoking;
	uint8_t peerInterested;

	//=== INTERNAL USE ONLY ===
	pthread_mutex_t syncMutex;
	pthread_cond_t syncCondvar;

} tcppeer_t;



#endif /* SWARM_TCPPEER_H_ */
