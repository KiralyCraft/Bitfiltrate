/*
 * tcpswarm.c
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#include "tcppeer/tcppeer.h"
#include "tcpswarm.h"
#include "tcpswarm_comm.h"
#include "tcpswarm_proto.h"
#include "tcpswarm_filters.h"

#include "../../network/conpool.h"

#include "../swarm_filters.h"
#include "../swarm_query.h"

#include <stdlib.h>
#include <pthread.h>

void* _tcpswarm_peerQueryFunction(void* __theImplementedPeer, swarm_query_type_e __queryType, void* __querySpecificArguments);
swarm_filters_peerdata_t* _tcpswarm_peerFilterFunction(dlinkedlist_t* __thePeerList, swarm_filters_peerdata_criteria_t* __theFilterCriteria);

swarm_definition_t* tcpswarm_createDefinition(void (*__thePostProcessingFunction)(void*))
{
	swarm_definition_t* _theSwarmDefinition = malloc(sizeof(swarm_definition_t));

	_theSwarmDefinition->incomingFunction = _tcpswarm_incomingFunction;
	_theSwarmDefinition->outgoingFunction = _tcpswarm_outgoingFunction;
	_theSwarmDefinition->processingFunction = _tcpswarm_processingFunction;
	_theSwarmDefinition->postProcessingFunction = __thePostProcessingFunction;

	_theSwarmDefinition->peerIngestFunction = _tcpswarm_peerIngestFunction;

	_theSwarmDefinition->generateMessageType = _tcpswarm_generatePacket;
	_theSwarmDefinition->peerQueueOutgoingPacket = _tcpswarm_peerQueueOutgoingPacket;

	_theSwarmDefinition->filterPeers = _tcpswarm_peerFilterFunction;

	_theSwarmDefinition->peerQueryFunction = _tcpswarm_peerQueryFunction;

	return _theSwarmDefinition;
}

void* _tcpswarm_peerIngestFunction(peer_networkconfig_h* __thePeerConnectionDetails,swarm_definition_t* __theSwarmDefinition, conpool_t* __theConnectionPool)
{
	//=== INITIALISE PEER-SPECIFIC FIELDS ===
	tcppeer_t* _theTCPPeer = malloc(sizeof(tcppeer_t));

	_theTCPPeer -> amChocking = 1;
	_theTCPPeer -> amInterested = 0;
	_theTCPPeer -> peerChoking = 1;
	_theTCPPeer -> peerInterested = 0; //TODO move these things to the peer class itself, along with the initialization of other mutexes
	_theTCPPeer -> peerNetworkConfig = __thePeerConnectionDetails;
	_theTCPPeer -> peerBitfield = dlinkedlist_createList();
	_theTCPPeer -> packetsReceivedBitfield = 0;
	conc_queue_init(&(_theTCPPeer -> peerIncomingBlockData));
	pthread_mutex_init(&(_theTCPPeer ->bitfieldMutex),NULL);
	pthread_mutex_init(&(_theTCPPeer->syncMutex),NULL);
	pthread_cond_init(&(_theTCPPeer->syncCondvar),NULL);

	//=== INITIALISE COMMUNICATION INTERNALS ===
	uint8_t _peerInitializationResult = _tcpswarm_peerInitialize(_theTCPPeer);
	if (_peerInitializationResult == 0)
	{
		free(_theTCPPeer);
		return NULL;
	}

	//TODO we're passing the tcp peer twice here, is this really the right approach? once as a socket descriptor, once as the execution context for processing
	//minute contemplation, i think it is. this is how we've designed the connection pool
	void* _thePeerCommunicationQueue = conpool_createConnection(__theConnectionPool,_theTCPPeer,__theSwarmDefinition->outgoingFunction,__theSwarmDefinition->incomingFunction,__theSwarmDefinition->processingFunction,_theTCPPeer,__theSwarmDefinition->postProcessingFunction);
	if (_thePeerCommunicationQueue == NULL)
	{
		//TODO failed, proper cleanup
		free(_theTCPPeer);
		return NULL;
	}
	_theTCPPeer -> peerOutgoingPacketQueue = _thePeerCommunicationQueue;
	return _theTCPPeer;
}

/*
 * This function assumes it has exclusive access to the peer data, such that list is not modified during iteraiton.
 *
 * Whoever handles our results should also make sure to clean them.
 */
swarm_filters_peerdata_t* _tcpswarm_peerFilterFunction(dlinkedlist_t* __thePeerList, swarm_filters_peerdata_criteria_t* __theFilterCriteria)
{
	swarm_filters_peerdata_t* _theFilteredPeers = swarm_filters_createPeerFilterBucket();

	if (__theFilterCriteria->peerFilterCriteria == SWARM_PEERFILTER_NETWORKSTATUS)
	{
		uint8_t _filterResult = tcpswarm_filters_filterByNetworkStatus(__thePeerList,_theFilteredPeers,__theFilterCriteria->peerFilterData);
		if (_filterResult == 0)
		{
			swarm_filters_destroyPeerFilterBucket(_theFilteredPeers);
			return NULL;
		}
	}
	else if (__theFilterCriteria->peerFilterCriteria == SWARM_PEERFILTER_PACKETTIME_INCOMING)
	{
		uint8_t _filterResult = tcpswarm_filters_filterByIncomingPacketTime(__thePeerList,_theFilteredPeers,__theFilterCriteria->peerFilterData);
		if (_filterResult == 0)
		{
			swarm_filters_destroyPeerFilterBucket(_theFilteredPeers);
			return NULL;
		}
	}
	else if (__theFilterCriteria->peerFilterCriteria == SWARM_PEERFILTER_PACKETTIME_OUTGOING)
	{
		uint8_t _filterResult = tcpswarm_filters_filterByOutgoingPacketTime(__thePeerList,_theFilteredPeers,__theFilterCriteria->peerFilterData);
		if (_filterResult == 0)
		{
			swarm_filters_destroyPeerFilterBucket(_theFilteredPeers);
			return NULL;
		}
	}
	else if (__theFilterCriteria->peerFilterCriteria == SWARM_PEERFILTER_PACKETCOUNT_INCOMING_NONZERO)
	{
		uint8_t _filterResult = tcpswarm_filters_filterByNonzeroIncomingPacketCount(__thePeerList,_theFilteredPeers,__theFilterCriteria->peerFilterData);
		if (_filterResult == 0)
		{
			swarm_filters_destroyPeerFilterBucket(_theFilteredPeers);
			return NULL;
		}
	}
	else //Consider unimplemented criteria
	{
		swarm_filters_destroyPeerFilterBucket(_theFilteredPeers);
		return NULL;
	}
	return _theFilteredPeers;

//	ai ramas aici, unde daca venea un pachet de un anumit id, setai bitul ala din packetsreceivebitfield.
//		tot asa, trebuie sa dai nitialize la byte-ul ala cu 0 cand creezi tcppeer-ul
}

/*
 * This method should process query targeted at the given peer, considering the given arguments, and return a result accordingly.
 *
 * This method is NOT THREAD SAFE, and the caller should make sure to at least keep the peer a valid reference. If more
 * synchronization is required, this method should assume it is being called by multiple threads.
 */
void* _tcpswarm_peerQueryFunction(void* __theImplementedPeer, swarm_query_type_e __queryType, void* __querySpecificArguments)
{
	tcppeer_t* _thePeer = __theImplementedPeer;
	if (__queryType == SWARM_QUERY_PIECE_COUNT) //Argument = NULL
	{
		size_t* _toReturn = malloc(sizeof(size_t));
		_toReturn[0] = conc_queue_count(_thePeer->peerIncomingBlockData);
		return _toReturn;
	}
	else if (__queryType == SWARM_QUERY_HAS_PIECE) //Argument = size_t*
	{
		size_t _requestedPieceIndex = *((size_t*)__querySpecificArguments);
		uint8_t* _toReturn = malloc(sizeof(uint8_t));
		_toReturn[0] = tcppeer_hasPiece(_thePeer,_requestedPieceIndex);
		return _toReturn;

	}
	else if (__queryType == SWARM_QUERY_DRAIN_PIECE) //Argument = NULL
	{
		return conc_queue_popifpossible(_thePeer->peerIncomingBlockData);
	}

	return NULL;
}
