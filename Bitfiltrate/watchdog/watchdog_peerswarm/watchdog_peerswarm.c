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

void _watchdog_peerswarm_executor(void* __watchdogContext);

watchdog_peerswarm_t* watchdog_peerswarm_init(watchdog_t* __theWatchdog,torrent_t* __torrentHash,conpool_t* __theConnectionPool)
{
	//=== SETTING UP THE TCP SWARM ===
	swarm_definition_t* _theSwarmDefinition = tcpswarm_createDefinition(swarm_postProcessPeerData);
	swarm_t* _peerSwarm = swarm_createPeerSwarm(_theSwarmDefinition, __torrentHash, __theConnectionPool);

	//=== SETTING UP THE WATCHDOG CONTEXT ===
	watchdog_peerswarm_t* _newPeerSwarmWatchdog = malloc(sizeof(watchdog_peerswarm_t));
	_newPeerSwarmWatchdog -> thePeerSwarm = _peerSwarm;

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

	//TODO make this thing wait on a conditional variable for modifications. Also submit this conditonal variable to the swarm itself.
	uint32_t _pendingPeerCount = conc_queue_count(_watchdogData->peerIngestionQueue);
	if (_pendingPeerCount > 0)
	{
		printf("WATCHDOG popping peer for ingestion\n");
		peer_networkconfig_h* _poppedPeerConfiguration = conc_queue_pop(_watchdogData->peerIngestionQueue);
		swarm_ingestPeer(_watchdogData->thePeerSwarm,_poppedPeerConfiguration); //This method is thread safe

		//TODO process packets which the swarm itself has post-processed

		//TODO unlock the swarm
	}
//	this method should push peers onto the swarm, where they get a connection craeted based onthe definition
}

void watchdog_peerswarm_ingestPeer(watchdog_peerswarm_t* __thePeerSwarmWatchdog, peer_networkconfig_h* __peerConfig,torrent_t* __torrentData)
{
	printf("WATCHDOG: Ingesting peer\n");
	//The concurrent queue always locks and is, as the name suggests, concurrent.
	conc_queue_push(__thePeerSwarmWatchdog->peerIngestionQueue,__peerConfig);
}
