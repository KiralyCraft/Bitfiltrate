/*
 * main.c
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#include "torrentinfo/torrentinfo.h"
#include "network/conpool.h"
#include "concurrent_queue.h"
#include "watchdog/watchdog.h"
#include "watchdog/watchdog_udptracker/watchdog_udptracker.h"
#include "watchdog/watchdog_peerswarm/watchdog_peerswarm.h"
#include "watchdog/watchdog_diskio/watchdog_diskio.h"

#include "swarm/swarm.h"
#include "swarm/tcpswarm/tcpswarm.h"

#include "peerfeed/peerfeed.h"
#include <signal.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	if (argc < 5)
	{
		printf("USAGE: [infohash] [startingpiecesizelog2] [mode] [peer_source] [tracker_port].\n");
		printf("The infohash must be an exact 40-charcater, hexadecimal lowercase string representing the torrent hash.\n");
		printf("The startingpiecesizelog2 is the upper range from which pieces are guessed. Default is 24.\n");
		printf("The mode must be explicitely a 1 or 0, indicated a file mode operation or UDP tracker respectively.\n");
		printf("The peer source can either be a filename (for mode 1), or a DNS entry of a UDP tracker. In this case, the port is also required.\n");
		exit(1);
	}

	if (argv[3][0] == '0' && argc < 6)
	{
		printf("The tracker port is also required.\n");
		exit(1);
	}

	watchdog_t* _theWatchdog = watchdog_createWatchdogSupervisor();
//
	conpool_t* theConnectionPool = conpool_createPool();
	conpool_t* theSwarmConnectionPool = conpool_createPool();
//	torrent_t* _theTorrent = torrent_dummyHashTorrent("2e299b0bf7af4aa3bc5213be50fcb6b7189a8e70");
	torrent_t* _theTorrent = torrent_dummyHashTorrent(argv[1]);

	piecetracker_t* _thePieceTracker = piecetracker_constructTracker();
	watchdog_peerswarm_t* _thePeerSwarm = watchdog_peerswarm_init(_theWatchdog,atoi(argv[2]),_theTorrent,theSwarmConnectionPool,_thePieceTracker);

	if (argv[3][0] == '1')
	{
		peerfeed_ingestPeersFromFile(_thePeerSwarm->peerIngestionQueue,argv[4]);
	}

	watchdog_diskio_t* _theDiskioWatchdog = watchdog_diskio_init(_theWatchdog,_thePieceTracker);

	if (argv[3][0] == '0')
	{
		watchdog_udptracker_init(_theTorrent,argv[4],atoi(argv[5]),_theWatchdog,_thePeerSwarm,theConnectionPool);
	}

	getchar();

}
