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
#include "dlinkedlist.h"

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

	/*
	 * This bitfield list contains dynamically allocated bytes that should be manually freed
	 * when this peer is no longer useful, or upon destruction.
	 *
	 * The bytes stored in this representation of the bitfield are using the HOST representation.
	 * EG: Bit 0 of byte 0 is piece #0, bit 1 of byte 0 is piece #1, and so on.
	 */
	dlinkedlist_t* peerBitfield;

	//=== INTERNAL USE ONLY ===
	pthread_mutex_t syncMutex;
	pthread_cond_t syncCondvar;

} tcppeer_t;



#endif /* SWARM_TCPPEER_H_ */
