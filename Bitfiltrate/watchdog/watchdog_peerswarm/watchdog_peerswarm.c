/*
 * watchdog_peerswarm.c
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#include "../../swarm/tcpswarm/tcpswarm.h"
#include "../../swarm/swarm.h"
#include "watchdog_peerswarm.h"
#include "../../torrentinfo/torrentinfo.h"
#include <stdlib.h>

#include "dlinkedlist.h"

void _watchdog_peerswarm_executor(void* __watchdogContext);

watchdog_peerswarm_t* watchdog_peerswarm_init(watchdog_t* __theWatchdog,torrent_t* __torrentHash,conpool_t* __theConnectionPool)
{
	//=== SETTING UP THE TCP SWARM ===
	swarm_definition_t* _theSwarmDefinition = tcpswarm_createDefinition(swarm_postProcessPeerData);
	swarm_t* _peerSwarm = swarm_createPeerSwarm(_theSwarmDefinition, __torrentHash, __theConnectionPool);

	//=== SETTING UP THE WATCHDOG CONTEXT ===
	watchdog_peerswarm_t* _newPeerSwarmWatchdog = malloc(sizeof(watchdog_peerswarm_t));
	_newPeerSwarmWatchdog -> thePeerSwarm = _peerSwarm;
	_newPeerSwarmWatchdog -> swamExecutionMode = SWARM_EXEC_GUESS_PIECE_SIZE;
//	_newPeerSwarmWatchdog -> swamExecutionMode = SWARM_EXEC_GUESS_PIECE_SIZE;
	conc_queue_init(&_newPeerSwarmWatchdog -> peerIngestionQueue);

	//=== SUBMITTING THE WATCHDOG ===
	uint8_t _submissionResult = watchdog_submitWatchdog(_newPeerSwarmWatchdog,_watchdog_peerswarm_executor,__theWatchdog);

	if (_submissionResult == 0)
	{
		//TODO handle error
	}
	return _newPeerSwarmWatchdog;
}

/*
 * This is a function which will be repeatedly called by a thread of the watchdog.
 *
 * This function will always be executed asynchronously with everything else.
 */
void _watchdog_peerswarm_executor(void* __watchdogContext)
{
	watchdog_peerswarm_t* _watchdogData = __watchdogContext;
	//Fetch replies from here
	uint32_t _pendingPeerCount = conc_queue_count(_watchdogData->peerIngestionQueue);
	if (_pendingPeerCount > 0)
	{
		printf("WATCHDOG popping peer for ingestion\n");
		peer_networkconfig_h* _poppedPeerConfiguration = conc_queue_pop(_watchdogData->peerIngestionQueue);
		swarm_ingestPeer(_watchdogData->thePeerSwarm,_poppedPeerConfiguration); //This method is thread safe
	}

	//=== SWARM EXECUTION MODES ===
	watchdog_peerswarm_execution_mode_e _theSwarmExecutionMode = _watchdogData->swamExecutionMode;
	if (_theSwarmExecutionMode == SWARM_EXEC_GUESS_PIECE_SIZE)
	{

		//=== FIGURING OUT CONNECTED PEER COUNT ===
		peer_networkconfig_status_h _desiredPeerNetworkStatus = PEER_CONNECTED;
		swarm_filters_peerdata_criteria_t _peerConnectedFilter;
		_peerConnectedFilter.peerFilterCriteria = SWARM_PEERFILTER_NETWORKSTATUS;
		_peerConnectedFilter.peerFilterData = &_desiredPeerNetworkStatus;

		swarm_filters_peerdata_t* _filteredConnectedPeers = swarm_filterPeer(_watchdogData->thePeerSwarm,&_peerConnectedFilter);
		if (_filteredConnectedPeers == NULL)
		{
			printf("Failed when filtering connected peers\n");
			return;
		}
		size_t _connectedPeers = dlinkedlist_getCount(_filteredConnectedPeers->peerData);
		swarm_filters_destroyPeerFilterBucket(_filteredConnectedPeers);

		//=== SENDING OUT REQUESTS FOR THE CURRENT PIECE SIZE ===

		//TODO send requests and mark the time when you sent them

		//=== SWITCH THE OPERATION IN CONFIRM MODE ===

		//TODO switch execution mode

	}
	else if (_theSwarmExecutionMode == SWARM_EXEC_CONFIRM_PIECE_SIZE)
	{
		//TODO wait until 15 seconds have passed since the last request, then check the bitfields. if any received, set piece size and change to normal mode.
	//		swarm_message_e _desiredPacket = SWARM_MESSAGE_BITFIELD-2; //Have to do it like this, or use the actual packet ID
	//		swarm_filters_peerdata_criteria_t _peerFilter;
	//		_peerFilter.peerFilterCriteria = SWARM_PEERFILTER_PACKETCOUNT_INCOMING_NONZERO;
	//		_peerFilter.peerFilterData = &_desiredPacket;
	//
	//		swarm_filters_peerdata_t* _filteredPeers = swarm_filterPeer(_watchdogData->thePeerSwarm,&_peerFilter);
	//		printf("%d\n",dlinkedlist_getCount(_filteredPeers->peerData));
	//		swarm_filters_destroyPeerFilterBucket(_filteredPeers);
	}
}

void watchdog_peerswarm_ingestPeer(watchdog_peerswarm_t* __thePeerSwarmWatchdog, peer_networkconfig_h* __peerConfig,torrent_t* __torrentData)
{
	printf("WATCHDOG: Ingesting peer\n");
	//The concurrent queue always locks and is, as the name suggests, concurrent.
	conc_queue_push(__thePeerSwarmWatchdog->peerIngestionQueue,__peerConfig);
}
