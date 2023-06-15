/*
 * watchdog_udptracker.c
 *
 *  Created on: May 25, 2023
 *      Author: kiraly
 */

#include "watchdog_udptracker.h"
#include "../../udptracker/udptracker.h"
#include "../../udptracker/udptracker_proto.h"
#include "../../torrentinfo/torrentinfo.h"
#include "../../network/conpool.h"
#include "../watchdog.h"

#include "../../swarm/peer/peer_networkdetails.h"
#include "../../swarm/swarm.h"

#include <stdlib.h>

void _watchdog_udptracker_executor(void* __watchdogContext);

void watchdog_udptracker_init(torrent_t* __theTorrentData,const char* __givenTrackerURL,uint32_t __givenTrackerPort,watchdog_t* __theWatchdog,watchdog_peerswarm_t* __peerSwarm,conpool_t* __theConnectionPool)
{
	udptrack_t* _theTracker = udptracker_create(__givenTrackerURL,__givenTrackerPort);
	uint8_t _initResult = udptracker_initialize(_theTracker,__theConnectionPool);

	if (_initResult == 0)
	{
		//TODO handle error
	}

	watchdog_udptracker_t* _watchdogData = malloc(sizeof(watchdog_udptracker_t));
	_watchdogData->theTrackerConnection = _theTracker;
	_watchdogData->theTorrentData = __theTorrentData;
	_watchdogData->thePeerSwarmWatchdog = __peerSwarm;

	uint8_t _submissionResult = watchdog_submitWatchdog(_watchdogData,_watchdog_udptracker_executor,__theWatchdog);
	if (_submissionResult == 0)
	{
		//TODO handle error
	}
}

/*
 * This comparator assumes the first argument is the key that is statically searched for, and it expects
 * a tracker conversation on the second variable. It returns 1 when the given key matches the conversation's torrent id.
 */
uint8_t _watchdog_udptracker_comparator_conversation_torrentid(void* __theSearchedKey, void* __iteratedElement)
{
	udptrack_conversation_t* _iteratedConversation = __iteratedElement;
	int32_t _torrentID = *((int32_t*)__theSearchedKey);

	if (_torrentID == _iteratedConversation->externalIdentifier)
	{
		return 1;
	}
	return 0;
}

int f = 0;
/*
 * This is a function which will be repeatedly called by a thread of the watchdog.
 *
 * This function will always be executed asynchronously with everything else.
 */
void _watchdog_udptracker_executor(void* __watchdogContext)
{
	watchdog_udptracker_t* _watchdogData = __watchdogContext;
	udptrack_t* _trackerData = _watchdogData->theTrackerConnection;
	torrent_t* _torrentData = _watchdogData->theTorrentData;
	watchdog_peerswarm_t* _thePeerSwarmWatchdog = _watchdogData->thePeerSwarmWatchdog;

	//=== PREPARE FOR WATCHDOG PROCESSING ===
	pthread_mutex_lock(&_trackerData->lockingMutex);
	pthread_cond_wait(&_trackerData->updateCondvar, &_trackerData->lockingMutex);
	//=== PROCESS WATCHDOG ACTIONS ===

	if (_trackerData->trackerStatus == UDPTRACK_INITIALIZED)
	{
		if (f == 0)
		{
			int32_t _announceTransactionID = udptracker_proto_generateTransactionID();
			void* _announcePacket = udptracker_proto_requestAnnouncePacket(_trackerData->connectionID,_torrentData->torrentHash,_announceTransactionID, _trackerData->peerID, _trackerData->downloaded,_trackerData->left,_trackerData->uploaded,1234);
			uint8_t _conversationInitiatorResult = udptracker_beginConversation(_announcePacket,UDP_TRACKER_PACKET_ANNOUNCE,_announceTransactionID, _torrentData->uniqueIdentifier,_trackerData,0);
			if (_conversationInitiatorResult == 0)
			{
				//TODO handle error
			}

			f=1;
		}
		else
		{
			udptrack_conversation_t* _foundConversation;
			do
			{
				_foundConversation = dlinkedlist_getCustomElement(&_torrentData->uniqueIdentifier,_watchdog_udptracker_comparator_conversation_torrentid,_trackerData->trackerConversations);
				if (_foundConversation != NULL)
				{
//					printf("DEBUG: Fetched convo type: %d,%d\n",_foundConversation->conversationType,_foundConversation->converstationStatus);
					udptrack_packet_reply_t* _theReplyPacket = _foundConversation->supplementalResponseData;
					//=== PROCESSING OF DATA ===
					if (_foundConversation->converstationStatus == UDP_TRACKER_CONV_FINISHED)
					{
						//=== SUBMITTING PEERS FROM ANNOUNCE TO THE SWARM ===
						if (_foundConversation->conversationType == UDP_TRACKER_PACKET_ANNOUNCE)
						{
							//TODO handle the rest of the announce packet, such as the rennounce interval and everything else
							udptrack_packet_reply_data_announce_t* _announceReplyData = _theReplyPacket->packetData;
							for (uint32_t _peerIterator = 0;_peerIterator < _announceReplyData->thePeerCount; _peerIterator++)
							{
								udptrack_packet_reply_data_announce_peer_t _peerData = _announceReplyData->thePeerList[_peerIterator];

								peer_networkconfig_h* _peerNetworkConfig = peer_networkdetails_generatePeerDetails(_peerData.peerIP,_peerData.peerPort);
								watchdog_peerswarm_ingestPeer(_thePeerSwarmWatchdog,_peerNetworkConfig,_torrentData);
							}
						}
					}
					else
					{
						//TODO handle conversations that have not been finished, but rahter interrupted or timed out
					}

					//=== CLEANUP ===
					uint8_t _deletionSuccessful = dlinkedlist_deleteElement(_foundConversation,_trackerData->trackerConversations);
					if (_deletionSuccessful == 0)
					{
						//TODO handle deletion error
					}
					else
					{
						udptracker_proto_destroyReplyPacket(_theReplyPacket); //Destroy the reply packet contained within
						free(_foundConversation); //Destroy the conversation
						//TODO destroy conversations externally using an API from udptracker.h
					}
				}
			}
			while(_foundConversation != NULL);
		}
//
//		aici send announce
//		int32_t _scrapeTransactionID = udptracker_proto_generateTransactionID();
//		void* _scrapePacket = udptracker_proto_requestScrapePacket(_trackerData->connectionID,_torrentData->torrentHash,_scrapeTransactionID);
//		uint8_t _conversationInitiatorResult = udptracker_beginConversation(_scrapePacket,UDP_TRACKER_PACKET_SCRAPE,_scrapeTransactionID, _torrentData->uniqueIdentifier,_trackerData,0);
//
//		if (_conversationInitiatorResult == 0)
//		{
//			//TODO handle error
//		}
	}
	else
	{
		//TODO handle situation where the tracker has not yet been initialized
	}

	//================================
	pthread_mutex_unlock(&_trackerData->lockingMutex);
}
