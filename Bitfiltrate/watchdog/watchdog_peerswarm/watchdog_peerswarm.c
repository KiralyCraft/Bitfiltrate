/*
 * watchdog_peerswarm.c
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#include "../../swarm/tcpswarm/tcpswarm.h"
#include "../../swarm/swarm.h"
#include "../../swarm/swarm_actions.h"
#include "../../swarm/swarm_block.h"
#include "watchdog_peerswarm.h"
#include "../../torrentinfo/torrentinfo.h"
#include <stdlib.h>


#include "dlinkedlist.h"

#define SWARM_PIECESIZE_LOG2_START 19

void _watchdog_peerswarm_executor(void* __watchdogContext);

watchdog_peerswarm_t* watchdog_peerswarm_init(watchdog_t* __theWatchdog,torrent_t* __torrentHash,conpool_t* __theConnectionPool,piecetracker_t* __thePieceTracker)
{
	//=== SETTING UP THE TCP SWARM ===
	swarm_definition_t* _theSwarmDefinition = tcpswarm_createDefinition(swarm_postProcessPeerData);
	swarm_t* _peerSwarm = swarm_createPeerSwarm(_theSwarmDefinition, __torrentHash, __theConnectionPool);

	//=== SETTING UP THE WATCHDOG CONTEXT ===
	watchdog_peerswarm_t* _newPeerSwarmWatchdog = malloc(sizeof(watchdog_peerswarm_t));
	_newPeerSwarmWatchdog -> thePeerSwarm = _peerSwarm;
	_newPeerSwarmWatchdog -> swamExecutionMode = SWARM_EXEC_GUESS_PIECE_SIZE;
	_newPeerSwarmWatchdog -> swarmPieceSize = 19;
	_newPeerSwarmWatchdog -> thePieceTracker = __thePieceTracker;
	_newPeerSwarmWatchdog -> lastWishedBlock = NULL;
	_newPeerSwarmWatchdog -> timeGuessedPiece = 0;
	_newPeerSwarmWatchdog -> timeLastPeerIngested = 0;
	_newPeerSwarmWatchdog -> timeWishLastFulfilled = 0;
	_newPeerSwarmWatchdog -> timeLastBitfieldUpdate = 0;
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


			if (_peerReceivedDataCount == 0)
			{
				printf("DEBUG: The piece size isn't %d, decreasing\n",_watchdogData->swarmPieceSize);
				_watchdogData->swarmPieceSize--;
				_watchdogData->swamExecutionMode = SWARM_EXEC_GUESS_PIECE_SIZE;
			}
			else
			{
				printf("log2 Piece size confirmed: %d. Draining peers!\n",_watchdogData->swarmPieceSize);
				piecetracker_setPieceSize(_watchdogData->thePieceTracker, _watchdogData->swarmPieceSize);

				//=== CLEANING PEERS OF RESIDUAL DATA ===

				size_t _peersWithData = dlinkedlist_getCount(_filteredPeers->peerData);
				for (size_t _connectedPeerIterator = 0; _connectedPeerIterator < _peersWithData; _connectedPeerIterator++)
				{
					void* _theDirtyPeer = dlinkedlist_getPosition(_connectedPeerIterator,_filteredPeers->peerData);
					while(1)
					{
						void* theData = swarm_query_peer(_watchdogData->thePeerSwarm,_theDirtyPeer,SWARM_QUERY_DRAIN_PIECE,NULL);

						if (theData == NULL)
						{
							printf("DEBUG: Peer %lu drained of test data\n",_connectedPeerIterator);
							break;
						}
						swarm_block_destroyBlockDataAndWrapper(theData);
					}
				}

				//=======================================
				_watchdogData->swamExecutionMode = SWARM_EXEC_DOWNLOAD;
			}
			swarm_filters_destroyPeerFilterBucket(_filteredPeers);
		}
	}
	else if (_theSwarmExecutionMode == SWARM_EXEC_DOWNLOAD)
	{
//		undeva aici in download e leak-ul
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
		//=== GETTING WISHLIST FROM PIECETRACKER ===
		uint8_t _areWeWishing = 0;
		piecetracker_wishlist_t* _wishedBlock = NULL;

		piecetracker_status_e _currentPieceTrackerStatus = piecetracker_getCurrentStatus(_watchdogData->thePieceTracker);
		if (_currentPieceTrackerStatus == PIECETRACKER_OPERATIONAL)
		{
			if (piecetracker_areAllPiecesUsed(_watchdogData->thePieceTracker) == 1 && difftime(time(NULL),_watchdogData->timeLastBitfieldUpdate) > 60)
			{
				printf("DEBUG: All pieces seem to be done, and no bifield updates happened for some time. Assuming complete!\n");
				_watchdogData->swamExecutionMode = SWARM_EXEC_COMPLETE;
				swarm_filters_destroyPeerFilterBucket(_filteredConnectedPeers);
				return;
			}

			_wishedBlock = piecetracker_getWish(_watchdogData->thePieceTracker);
			if (_watchdogData->lastWishedBlock == NULL)
			{
				_watchdogData->lastWishedBlock = _wishedBlock;
				_areWeWishing = 1;
			}
			else
			{

				uint8_t _isLastWishExpired = 0;
				if (difftime(time(NULL),_watchdogData->timeWishLastFulfilled) > 10)
				{
					_isLastWishExpired = 1;
					printf("DEBUG: Last wish has expired, requesting data again\n");
				}

				if (_isLastWishExpired || piecetracker_isWishFulfilled(_watchdogData->thePieceTracker,_watchdogData->lastWishedBlock))
				{
					_watchdogData->timeWishLastFulfilled = time(NULL);
					piecetracker_destroyWish(_watchdogData->lastWishedBlock); //Once the old wish has been fulfilled, destroy it.

					_watchdogData->lastWishedBlock = _wishedBlock;
					_areWeWishing = 1;
				}
				else //If not expired and not fulfilled
				{
					piecetracker_destroyWish(_wishedBlock);
				}
			}
//				printf("DEBUG: Wished block: %lu %lu %lu\n",_wishedBlock->wishedPieceIndex,_wishedBlock->wishedBlockIndex,_wishedBlock->wishedBlockSize);
		}

		//=== ITERATING FILTERED PEERS ===
		uint8_t _hasRequestedFromPeers = 0;
		size_t _connectedPeerCount = dlinkedlist_getCount(_filteredConnectedPeers->peerData);

		for (size_t _connectedPeerIterator = 0; _connectedPeerIterator < _connectedPeerCount; _connectedPeerIterator++)
		{
			void* _theConnectedIteratedPeer = dlinkedlist_getPosition(_connectedPeerIterator,_filteredConnectedPeers->peerData);

			//=== UPDATING PIECE STATUS ===
			size_t* _queriedData = swarm_query_peer(_watchdogData->thePeerSwarm,_theConnectedIteratedPeer,SWARM_QUERY_HIGHEST_BITFIELD_PIECE,NULL);

			if (_queriedData != NULL)
			{
				; //TODO handle exception
			}

			uint8_t _updateResult = piecetracker_setMaximumPieceCount(_watchdogData->thePieceTracker,(*_queriedData)+1); //Add one, because the query returns an index, not a count
			if (_updateResult == 1)
			{
				_watchdogData->timeLastBitfieldUpdate = time(NULL);
				printf("DEBUG: Watchdog updated max piece count of piece tracker!\n");
			}
			free(_queriedData); //TODO move this in a better manner, not like this

			//=== REQUESTING DATA IF POSSIBLE===
			if (_areWeWishing == 1 && _wishedBlock != NULL) //If we have a wish and are actually wishing
			{
//				printf("Can request data! %lu\n",_timeOfCheck);

				uint8_t _resultInformInterested = swarm_informInterestedPeer(_watchdogData->thePeerSwarm,_theConnectedIteratedPeer);
				if (_resultInformInterested == 0)
				{
					printf("Failed to inform interested peer when downloading\n");
					continue;
				}
				//=== SPECIFYING REQUEST DETAILS ===

				size_t _wishedBlockIndexStart = _wishedBlock->wishedBlockIndexStart;
				size_t _wishedBlockIndexEnd = _wishedBlock->wishedBlockIndexEnd;
				printf("DEBUG: wishing for piece %lu\n",_wishedBlock->wishedPieceIndex);
				for (size_t _blockWishIterator = _wishedBlockIndexStart; _blockWishIterator <= _wishedBlockIndexEnd; _blockWishIterator++)
				{
					uint8_t _requestResult = swarm_requestPiece(_watchdogData->thePeerSwarm,_theConnectedIteratedPeer,
							_wishedBlock->wishedPieceIndex,
							_blockWishIterator*(_wishedBlock->wishedBlockSize),
							_wishedBlock->wishedBlockSize); //TODO is this really the right place to request packets from? does it need to be thread safe?

					if (_requestResult == 0)
					{
						printf("Failed to request peice from peer when downloading\n");
						continue;
					}
				}
				//=== NOTE THAT REQUESTS FOR DATA WERE MADE ===
				_hasRequestedFromPeers = 1;
			}

			//=== INGESTING EXISTING QUEUE FROM THIS PEER ===
			while(1)
			{
				void* theData = swarm_query_peer(_watchdogData->thePeerSwarm,_theConnectedIteratedPeer,SWARM_QUERY_DRAIN_PIECE,NULL);

				if (theData == NULL)
				{
//					printf("DEBUG: Peer %lu drained of actual data\n",_connectedPeerIterator);
					break;
				}

				uint8_t _pieceIngestResult = piecetracker_ingestSwarmBlock(_watchdogData->thePieceTracker,theData);
				if (_pieceIngestResult == 0)
				{
					;//TODO handle situation, the block may just already be full
//					printf("Failed to ingest peice from peer when downloading, probably block has been used already\n");
				}

				swarm_block_destroyBlockWrapper(theData); //Only destroy the block wrapper, as the data will still be used inside the piece tracker.
			}
		}
//		//=== UPDATE LAST TIME SENT REQUESTS ===
//		if (_hasRequestedFromPeers == 1)
//		{
//			_watchdogData->timeLastRequestedActualBlock = _timeOfCheck;
//		}
		//=== CLEANUP ===
//		if (_wishedBlock != NULL) piecetracker_destroyWish(_wishedBlock);
		swarm_filters_destroyPeerFilterBucket(_filteredConnectedPeers);
	}
	else if (_theSwarmExecutionMode == SWARM_EXEC_COMPLETE)
	{
		printf("DEBUG: Download complete! :)\n");
	}
}


void watchdog_peerswarm_ingestPeer(watchdog_peerswarm_t* __thePeerSwarmWatchdog, peer_networkconfig_h* __peerConfig,torrent_t* __torrentData)
{
	printf("WATCHDOG: Ingesting peer\n");
	//The concurrent queue always locks and is, as the name suggests, concurrent.
	conc_queue_push(__thePeerSwarmWatchdog->peerIngestionQueue,__peerConfig);
}
