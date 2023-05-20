/*
 * main.c
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#include "torrentinfo/torrentinfo.h"
//#include "network/protocols/tcp_demo/tcp_demo.h"
//#include "network/protocols/udp_demo/udp_demo.h"
#include "network/conpool.h"
#include "concurrent_queue.h"

#include "udptracker/udptracker.h"

int main()
{
//	torrent_t* _theTorrent = openTorrent("systemrescue-10.00-amd64.iso.torrent");
//
//	//========DEBUG CODE BELOW==========
//	int createdSocket = udp_conn_createSocket();
//
//	conpool_t* theConnectionPool = conpool_createPool();
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

	udptracker_connect("fosstorrents.com",6969);
}
