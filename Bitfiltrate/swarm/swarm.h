/*
 * swarm.h
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#ifndef SWARM_POOL_SWARM_H_
#define SWARM_POOL_SWARM_H_

#include <stdint.h>
#include "../network/conpool.h"
#include "../torrentinfo/torrentinfo.h"
#include "peer/peer_networkdetails.h"
#include "dlinkedlist.h"

typedef struct
{
	/*
	 * This is an implementation-specific function that generates peer data representations.
	 */
	void* (*peerIngestFunction)(void*);

} swarm_definition_t;

typedef struct
{
	//=== TORRENT-RELATED ===
	uint8_t torrentHash[20];
	//=== SWARM-RELATED ===
	/*
	 * Store the definition within the swarm itself.
	 */
	swarm_definition_t* currentSwarmDefinition;
	/*
	 * This list contains data corresponding to the specific implementation of the peers, and is populated
	 * upon ingestion, based on the particular definition of the swarm.
	 */
	dlinkedlist_t* currentPeerData;

} swarm_t;

swarm_t* swarm_createPeerSwarm(swarm_definition_t* __theSwarmDefinition,torrent_t* __theTorrentInfo,conpool_t* __connectionPool);

/*
 * This method ingests a peer, represented solely by it's network details, into the swarm.
 * The way by which this peer is communicated with is up to the swarm parameters, and particularly the swarm definition used to create it.
 */
uint8_t swarm_ingestPeer(swarm_t* __theSwarm, peer_networkconfig_h* __peerDetails);

#endif /* SWARM_POOL_SWARM_H_ */
