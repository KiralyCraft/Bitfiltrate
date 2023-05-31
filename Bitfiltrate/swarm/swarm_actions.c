/*
 * swarm_actions.c
 *
 *  Created on: May 31, 2023
 *      Author: kiraly
 */

#include "swarm_message_types.h"
#include "swarm_actions.h"
#include "dlinkedlist.h"

//aici faceam un swarm_connectAll() (pentru handshake de exemplu); singurele alte chestii specifice ar fi swarm_cleanupAll(), sawrm_destoryAll(), swarm_broadcast(), si apoi cele pentru mesaje efective

/*
 * This method is NOT THREAD SAFE.
 */
uint8_t swarm_connectPeer(swarm_t* __theSwarm,void* __genericPeerRepresentation)
{
	swarm_definition_t* _currentSwarmDefinition = __theSwarm ->currentSwarmDefinition;
	void* _theHandshakeMessage = _currentSwarmDefinition->generateMessageType(SWARM_MESSAGE_HANDSHAKE, __theSwarm->torrentHash);
	uint8_t _queueResult = _currentSwarmDefinition->peerQueueOutgoingPacket(_theHandshakeMessage,__genericPeerRepresentation);
	return _queueResult;
}

size_t swarm_connectAll(swarm_t* __theSwarm)
{
	swarm_definition_t* _currentSwarmDefinition = __theSwarm ->currentSwarmDefinition;
	void* _theHandshakeMessage = _currentSwarmDefinition->generateMessageType(SWARM_MESSAGE_HANDSHAKE, __theSwarm->torrentHash);
	return swarm_broadcast(__theSwarm,_theHandshakeMessage);
}

/*
 * This particular implementation of the function is THREAD SAFE.
 */
size_t swarm_broadcast(swarm_t* __theSwarm,void* __genericPacketData)
{
	size_t _currentFailures = 0;
	dlinkedlist_t* _currentPeerSwarm = __theSwarm -> currentPeerData;
	swarm_definition_t* _currentSwarmDefinition = __theSwarm ->currentSwarmDefinition;

	pthread_mutex_lock(&(__theSwarm->peerManipulationMutex));
	//TODO optimize the iteration of these peers, because currently the efficiency is n^2
	size_t _currentPeerSwarmSize = dlinkedlist_getCount(_currentPeerSwarm);
	for (size_t _currentPeerIndex = 0; _currentPeerIndex < _currentPeerSwarmSize; _currentPeerIndex++)
	{
		void* _currentPeer = dlinkedlist_getPosition(_currentPeerIndex,_currentPeerSwarm);
		uint8_t queueResult = _currentSwarmDefinition->peerQueueOutgoingPacket(__genericPacketData,_currentPeer);
		if (queueResult == 0)
		{
			_currentFailures++;
		}
	}
	pthread_mutex_unlock(&(__theSwarm->peerManipulationMutex));
	return _currentFailures;
}
