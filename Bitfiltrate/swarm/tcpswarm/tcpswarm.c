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
#include <stdlib.h>
#include "../../network/conpool.h"
#include <pthread.h>

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
	conc_queue_init(&(_theTCPPeer -> peerIncomingPieceData));
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



