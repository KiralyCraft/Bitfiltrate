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
#include <stdlib.h>

void _watchdog_udptracker_executor(void* __watchdogContext);

void watchdog_udptracker_init(torrent_t* __theTorrentData,const char* __givenTrackerURL,uint32_t __givenTrackerPort,watchdog_t* __theWatchdog,conpool_t* __theConnectionPool)
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

	uint8_t _submissionResult = watchdog_submitWatchdog(_watchdogData,_watchdog_udptracker_executor,__theWatchdog);
	if (_submissionResult == 0)
	{
		//TODO handle error
	}
}

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

	//=== PREPARE FOR WATCHDOG PROCESSING ===
	pthread_mutex_lock(&_trackerData->lockingMutex);
	pthread_cond_wait(&_trackerData->updateCondvar, &_trackerData->lockingMutex);
	//=== PROCESS WATCHDOG ACTIONS ===

	if (_trackerData->trackerStatus == UDPTRACK_INITIALIZED)
	{

		aici send announce
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
