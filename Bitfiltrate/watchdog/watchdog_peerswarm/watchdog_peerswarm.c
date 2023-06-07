/*
 * watchdog_peerswarm.c
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#include "../../swarm/tcpswarm/tcpswarm.h"
#include "../../swarm/swarm.h"
#include "../../swarm/swarm_actions.h"
#include "watchdog_peerswarm.h"
#include "../../torrentinfo/torrentinfo.h"
#include <stdlib.h>


#include "dlinkedlist.h"

#define SWARM_PIECESIZE_LOG2_START 19

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
	_newPeerSwarmWatchdog -> swarmPieceSize = 19;
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
		uint8_t _ingestResult = swarm_ingestPeer(_watchdogData->thePeerSwarm,_poppedPeerConfiguration); //This method is thread safe
		if (_ingestResult == 0)
		{
			; //TODO handle ingest failure
		}

		_watchdogData->timeLastPeerIngested = time(NULL);
	}

	//=== SWARM EXECUTION MODES ===
	watchdog_peerswarm_execution_mode_e _theSwarmExecutionMode = _watchdogData->swamExecutionMode;
	if (_theSwarmExecutionMode == SWARM_EXEC_GUESS_PIECE_SIZE)
	{
		if (difftime(time(NULL),_watchdogData->timeLastPeerIngested) < 5)
		{
			return;
		}
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
		size_t _connectedPeerCount = dlinkedlist_getCount(_filteredConnectedPeers->peerData);

		if (_connectedPeerCount > 0)
		{
			printf("DEBUG: Watchdog probing peers\n");
			//=== COMPUTING OFFSET ===
			uint32_t _guessedPieceSize = (1 << _watchdogData->swarmPieceSize);
			//=== SENDING OUT REQUESTS FOR THE CURRENT PIECE SIZE ===
			for (size_t _connectedPeerIterator = 0; _connectedPeerIterator < _connectedPeerCount; _connectedPeerIterator++)
			{
				void* _theConnectedIteratedPeer = dlinkedlist_getPosition(_connectedPeerIterator,_filteredConnectedPeers->peerData);
				//Request one byte before the theoretical end of the piece. If it's received, means the piece size is valid.
				uint8_t _resultInformInterested = swarm_informInterestedPeer(_watchdogData->thePeerSwarm,_theConnectedIteratedPeer);
				if (_resultInformInterested == 0)
				{
					printf("Failed to inform interested peer\n");
					//TODO Handle request errors
				}
				uint8_t _requestResult = swarm_requestPiece(_watchdogData->thePeerSwarm,_theConnectedIteratedPeer,0,_guessedPieceSize-1,1); //TODO is this really the right place to request packets from? does it need to be thread safe?
				if (_requestResult == 0)
				{
					printf("Failed to request peice from peer\n");
					//TODO Handle request errors
				}
			}
			_watchdogData->timeGuessedPiece = time(NULL);
			//=== SWITCH THE OPERATION IN CONFIRM MODE ===
			_watchdogData->swamExecutionMode = SWARM_EXEC_CONFIRM_PIECE_SIZE;
			printf("Told peers about pieces!\n");

		}
		swarm_filters_destroyPeerFilterBucket(_filteredConnectedPeers);
	}
	else if (_theSwarmExecutionMode == SWARM_EXEC_CONFIRM_PIECE_SIZE)
	{
		if (difftime(time(NULL),_watchdogData->timeGuessedPiece) >= 15)
		{
			swarm_message_e _desiredPacket = SWARM_MESSAGE_PIECE-2; //Have to do it like this, or use the actual packet ID
			swarm_filters_peerdata_criteria_t _peerFilter;
			_peerFilter.peerFilterCriteria = SWARM_PEERFILTER_PACKETCOUNT_INCOMING_NONZERO;
			_peerFilter.peerFilterData = &_desiredPacket;

			swarm_filters_peerdata_t* _filteredPeers = swarm_filterPeer(_watchdogData->thePeerSwarm,&_peerFilter);
			size_t _peerReceivedDataCount = dlinkedlist_getCount(_filteredPeers->peerData);
			printf("DEBUG WATCHDOG CONFIRM PIECE: Received data count %d\n",_peerReceivedDataCount);
			swarm_filters_destroyPeerFilterBucket(_filteredPeers);

			if (_peerReceivedDataCount == 0)
			{
				printf("DEBUG: The piece size isn't %d, decreasing\n",_watchdogData->swarmPieceSize);
				_watchdogData->swarmPieceSize--;
				_watchdogData->swamExecutionMode = SWARM_EXEC_GUESS_PIECE_SIZE;
			}
			else
			{
				printf("log2 Piece size confirmed! %d\n",_watchdogData->swarmPieceSize);
				_watchdogData->swamExecutionMode = SWARM_EXEC_DOWNLOAD;
			}
		}
	}
	else if (_theSwarmExecutionMode == SWARM_EXEC_DOWNLOAD)
	{
		//=== FILTERING THE PEERS, TAKING ONLY CONNECTED ONES ===
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
		//=== ITERATING FILTERED PEERS ===
		size_t _connectedPeerCount = dlinkedlist_getCount(_filteredConnectedPeers->peerData);
		for (size_t _connectedPeerIterator = 0; _connectedPeerIterator < _connectedPeerCount; _connectedPeerIterator++)
		{
			void* _theConnectedIteratedPeer = dlinkedlist_getPosition(_connectedPeerIterator,_filteredConnectedPeers->peerData);
			size_t* _queriedData = swarm_query_peer(_watchdogData->thePeerSwarm,_theConnectedIteratedPeer,SWARM_QUERY_PIECE_COUNT,NULL);
		}
		//=== CLEANUP THE FILTERS ===
		swarm_filters_destroyPeerFilterBucket(_filteredConnectedPeers);
//		apoi o chestie asemanatoare cu reconstruirea bitfieldului si a pieces
	}
}

void watchdog_peerswarm_ingestPeer(watchdog_peerswarm_t* __thePeerSwarmWatchdog, peer_networkconfig_h* __peerConfig,torrent_t* __torrentData)
{
	printf("WATCHDOG: Ingesting peer\n");
	//The concurrent queue always locks and is, as the name suggests, concurrent.
	conc_queue_push(__thePeerSwarmWatchdog->peerIngestionQueue,__peerConfig);
}
