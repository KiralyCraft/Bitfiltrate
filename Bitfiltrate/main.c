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

	watchdog_t* _theWatchdog = watchdog_createWatchdogSupervisor();
	conpool_t* theConnectionPool = conpool_createPool();
	torrent_t* _theTorrent = torrent_openTorrent("systemrescue-10.00-amd64.iso.torrent");
	watchdog_udptracker_init(_theTorrent,"tracker.openbittorrent.com",6969,_theWatchdog,theConnectionPool);

	getchar();
}
