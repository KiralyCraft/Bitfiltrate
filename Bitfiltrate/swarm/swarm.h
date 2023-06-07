/*
 * swarm.h
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#ifndef SWARM_POOL_SWARM_H_
#define SWARM_POOL_SWARM_H_

#include "../network/conpool.h"
#include "../torrentinfo/torrentinfo.h"
#include "peer/peer_networkdetails.h"
#include "dlinkedlist.h"
#include "swarm_message_types.h"
#include "swarm_filters.h"
#include "swarm_query.h"

#include <pthread.h>
#include <stdint.h>

typedef struct swarm_definition_t swarm_definition_t;

struct swarm_definition_t
{
	//==== DEFINITION API ====
	/*
	 * This is an implementation-specific function that generates peer data representations.
	 */
	void* (*peerIngestFunction)(peer_networkconfig_h*, swarm_definition_t*, conpool_t*);

	/*
	 * This is an implementation-specific function which submits a packet to a peer of the swarm.
	 *
	 * The swarm itself does not know how to operate this peer, but it only knows the general procedures.
	 * It is up to the definition to implement specifics, such as submitting a packet for the peer.
	 *
	 * This function should accept the packet to be sent as the first parameter, and the actual
	 * destination as the second. The destination is usually a peer of the swarm, which has been previously
	 * generated by this definition.
	 */
	uint8_t (*peerQueueOutgoingPacket)(void*,void*);

	/*
	 * This is an implemenation-specific function that the swarm definition should implement.
	 * Given that the swarm does not know the details of the underlying implementation, it asks
	 * the implementation for representation-specific details about different packet types, as given.
	 *
	 * Given that this procedure might accept data of various types, the second argument is up to the swarm to decide,
	 * and up to the implementation to interpret accordingly.
	 */
	void* (*generateMessageType)(swarm_message_e,void*);

	/*
	 * This function is used to filter the peers from the given swarm data according to the specified criteria.
	 */
	swarm_filters_peerdata_t* (*filterPeers)(dlinkedlist_t*,swarm_filters_peerdata_criteria_t*);

	/*
	 * This function is used to query given peers based on certain attributes.
	 * The first argument is the implementation specific representation of a peer, followed by the query type and the arguments that it may
	 * require.
	 *
	 * The success of this function is determined by the query type.
	 */
	void* (*peerQueryFunction)(void*,swarm_query_type_e,void*);
	//==== INTERNAL USE ====
	void (*outgoingFunction)(void*,void*,void*);
	void* (*incomingFunction)(void*,void*);
	void* (*processingFunction)(void*);
	//==== CALLBACKS ====
	void (*postProcessingFunction)(void*);

};

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

	/*
	 * This is the connection pool stored within the swarm itself upon creation.
	 * New peers are ingested into this connection pool.
	 */
	conpool_t* theConnectionPool;

	/*
	 * This is a mutex which should be acquired for peer manipulation purposes.
	 */
	pthread_mutex_t peerManipulationMutex;

} swarm_t;

swarm_t* swarm_createPeerSwarm(swarm_definition_t* __theSwarmDefinition,torrent_t* __theTorrentInfo,conpool_t* __connectionPool);

/*
 * This method ingests a peer, represented solely by it's network details, into the swarm.
 * The way by which this peer is communicated with is up to the swarm parameters, and particularly the swarm definition used to create it.
 * This is a blocking method, based on who calls it. This method is THREAD SAFE.
 */
uint8_t swarm_ingestPeer(swarm_t* __theSwarm, peer_networkconfig_h* __peerDetails);

/*
 * This method processes data for peers, regardless of the way they arrive. The data received as arguments comes after the packet has been
 * processed (by another entity, such as the specific swarm implementation) into a more universal language.
 */
void swarm_postProcessPeerData(void* __thePeerData);

/*
 * This method filters peers from the swarm based on their connectivity criteria. It also takes an optional argument,
 * which is up to the implementation to use.
 * If anything went wrong filtering the peers,this function returns NULL.
 *
 * In the event that no peer fits the filtering criteria, this method still retuns an object with the peerCount set to zero.
 *
 * Methods that call this function should clear the received package bundle.
 */
swarm_filters_peerdata_t* swarm_filterPeer(swarm_t* __theSwarm,swarm_filters_peerdata_criteria_t* __peerFilterCriteria);

/*
 * Used to query peers of this swarm about various things, and returns results accordingly.
 * Whether or not the query succeeded highly depends on the specific query type.
 *
 * Results returned by this function should be freed accordingly by the caller.
 *
 * This method is THREAD SAFE.
 */
void* swarm_query_peer(swarm_t* __theSwarm, void* __theImplementedPeer, swarm_query_type_e __queryType, void* __querySpecificArguments);


#endif /* SWARM_POOL_SWARM_H_ */
