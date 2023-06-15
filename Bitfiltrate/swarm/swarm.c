/*
 * swarm.c
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#include "swarm.h"
#include "../torrentinfo/torrentinfo.h"
#include "../network/conpool.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "swarm_actions.h"

swarm_t* swarm_createPeerSwarm(swarm_definition_t* __theSwarmDefinition,torrent_t* __theTorrentInfo,conpool_t* __connectionPool)
{
	swarm_t* _newSwarm = malloc(sizeof(swarm_t));
	memcpy(_newSwarm->torrentHash,__theTorrentInfo->torrentHash,sizeof(uint8_t) * 20);

	_newSwarm->currentSwarmDefinition = __theSwarmDefinition;
	_newSwarm->currentPeerData = dlinkedlist_createList();
	_newSwarm->theConnectionPool = __connectionPool;

	pthread_mutex_init(&_newSwarm->peerManipulationMutex,NULL);
//	aici am ramas, pentru create swarm cu connection pool si toate cele; connection pool vine din argumente
//	cand se face ingest la un peer, submit a new connection in pool care are un income function comun (tip network)
//	si un processing function de undeva din "protocol", care apoi aduce rezulatele aici dupa procesare, asemanator cu udp tracker.
//
//	la inceput, cand se face create la peer swarm, ar trebui sa se construiascsa si un anume bitfield sau infromatiile neceare.
//	acum ca vad, asta e de fapt echivalentul al udptracker, deci aici se face procesarea de la ce o trimis protocolul. astea de aici apoi dau
//	(sau tin) cumva la watchdog, ca sa scrie fisierele pe disk.
//
//	oare sa nu scrie de fapt astea pe disk cumva? direct de aici? vedem.

	return _newSwarm;
}
uint8_t swarm_ingestPeer(swarm_t* __theSwarm, peer_networkconfig_h* __peerDetails)
{
	pthread_mutex_lock(&__theSwarm->peerManipulationMutex);
	swarm_definition_t* _theSwarmDefinition = __theSwarm->currentSwarmDefinition;
	conpool_t* _theConnectionPool = __theSwarm->theConnectionPool;
	dlinkedlist_t* _thePeerData = __theSwarm->currentPeerData;
	//TODO check if the peer doesn't already exist
	void* _theImplementationSpecificPeerRepresentation = _theSwarmDefinition->peerIngestFunction(__peerDetails,_theSwarmDefinition,_theConnectionPool);
	uint8_t _insertionResult = dlinkedlist_insertElement(_theImplementationSpecificPeerRepresentation,_thePeerData);

	if (_insertionResult == 0)
	{
		pthread_mutex_unlock(&__theSwarm->peerManipulationMutex);
		//TODO dis-ingest the peer if it failed to be added
		return 0;
	}
	else
	{
		uint8_t _peerSubmissionResult = swarm_connectPeer(__theSwarm,_theImplementationSpecificPeerRepresentation);
		if (_peerSubmissionResult == 0)
		{
			pthread_mutex_unlock(&__theSwarm->peerManipulationMutex);
			printf("Aw no couldn't connect peer when ingesting :(\n");
			return 0;
		}

		uint8_t _peerUnchokeResult = swarm_informInterestedPeer(__theSwarm,_theImplementationSpecificPeerRepresentation);
		if (_peerUnchokeResult == 0)
		{
			pthread_mutex_unlock(&__theSwarm->peerManipulationMutex);
			printf("Aw no couldn't inform interested peer when ingesting :(\n");
			return 0;
		}
	}
	pthread_mutex_unlock(&__theSwarm->peerManipulationMutex);
	//printf("Peer ingested!\n");
	return 1;
}

void swarm_postProcessPeerData(void* __thePeerData)
{
	//TODO
}

/*
 * Filters peers of a swarm, in a locking way, based on the given criteria.
 * The peers returned in this bucket are safe to be iterated asynchronously, as they are just
 * the references to the peers contained within the pool, in an abstract way, depending on the implementation.
 */
swarm_filters_peerdata_t* swarm_filterPeer(swarm_t* __theSwarm,swarm_filters_peerdata_criteria_t* __peerFilterCriteria)
{
	pthread_mutex_lock(&__theSwarm->peerManipulationMutex);
	swarm_definition_t* _theSwarmDefinition = __theSwarm->currentSwarmDefinition;
	swarm_filters_peerdata_t* _filteredData = _theSwarmDefinition->filterPeers(__theSwarm->currentPeerData,__peerFilterCriteria);
	pthread_mutex_unlock(&__theSwarm->peerManipulationMutex);

	return _filteredData;
}

/*
 * Queries the given peer based on certain criteria.
 */
void* swarm_query_peer(swarm_t* __theSwarm, void* __theImplementedPeer, swarm_query_type_e __queryType, void* __querySpecificArguments)
{
	pthread_mutex_lock(&__theSwarm->peerManipulationMutex);
	swarm_definition_t* _theSwarmDefinition = __theSwarm->currentSwarmDefinition;
	void* _queriedData = _theSwarmDefinition->peerQueryFunction(__theImplementedPeer,__queryType,__querySpecificArguments);
	pthread_mutex_unlock(&__theSwarm->peerManipulationMutex);
	return _queriedData;

}
