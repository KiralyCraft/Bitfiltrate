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

int main()
{

//
//	//========DEBUG CODE BELOW==========
//	int createdSocket = udp_conn_createSocket();
//

//
//	conc_queue* outgoingPacketQueue = conpool_createConnection(theConnectionPool,createdSocket,udp_conn_outgoingFunction,udp_conn_incomingFunction,udp_conn_processingFunction);
//	getchar();
//
//	char buffer[50] = {0};
//	sprintf(buffer, "%s\n", "Macaroane cu branza baaa");
//
//	void* theBuiltPacket = udp_conn_buildPacket(buffer,sizeof(buffer),0);
//	conc_queue_push(outgoingPacketQueue,theBuiltPacket);
//	getchar();

//	udptrack_t* theTracker = udptracker_create("tracker.openbittorrent.com",6969);
//
//	udptracker_initialize(theTracker,theConnectionPool);

//	swarm_t* _thePeerSwarm = swarm_createPeerSwarm();

	//=========================
	watchdog_t* _theWatchdog = watchdog_createWatchdogSupervisor();
//
	conpool_t* theConnectionPool = conpool_createPool();
	conpool_t* theSwarmConnectionPool = conpool_createPool();
//	torrent_t* _theTorrent = torrent_openTorrent("systemrescue-10.01-amd64.iso.torrent");
	torrent_t* _theTorrent = torrent_dummyHashTorrent("84130aa9cb8503ad4330bb8e6c69129c1d2f4464");
	piecetracker_t* _thePieceTracker = piecetracker_constructTracker();
	watchdog_peerswarm_t* _thePeerSwarm = watchdog_peerswarm_init(_theWatchdog,_theTorrent,theSwarmConnectionPool,_thePieceTracker);

	peerfeed_ingestPeersFromFile(_thePeerSwarm->peerIngestionQueue,"peers.txt");
//
	watchdog_diskio_t* _theDiskioWatchdog = watchdog_diskio_init(_theWatchdog,_thePieceTracker);
//	watchdog_udptracker_init(_theTorrent,"tracker.openbittorrent.com",6969,_theWatchdog,_thePeerSwarm,theConnectionPool);

	getchar();
}
