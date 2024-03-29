/*
 * watchdog_udptracker.h
 *
 *  Created on: May 25, 2023
 *      Author: kiraly
 */

#ifndef WATCHDOG_WATCHDOG_UDPTRACKER_WATCHDOG_UDPTRACKER_H_
#define WATCHDOG_WATCHDOG_UDPTRACKER_WATCHDOG_UDPTRACKER_H_

#include "../../udptracker/udptracker.h"
#include "../../torrentinfo/torrentinfo.h"
#include "../watchdog.h"

#include "../watchdog_peerswarm/watchdog_peerswarm.h"

typedef struct
{
	udptrack_t* theTrackerConnection;
	torrent_t* theTorrentData;

	//=== SCRAPE INFO ===
	//TBD
	//=== PEER SWARM ===
	watchdog_peerswarm_t* thePeerSwarmWatchdog;
} watchdog_udptracker_t;

void watchdog_udptracker_init(torrent_t* __theTorrentData,const char* __givenTrackerURL,uint32_t __givenTrackerPort,watchdog_t* __theWatchdog,watchdog_peerswarm_t* __peerSwarm,conpool_t* __theConnectionPool);

#endif /* WATCHDOG_WATCHDOG_UDPTRACKER_WATCHDOG_UDPTRACKER_H_ */
