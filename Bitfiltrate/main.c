/*
 * main.c
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#include "torrentinfo/torrentinfo.h"
#include "network/protocols/tcp_demo/tcp_demo.h"
#include "network/conpool.h"
#include "concurrent_queue.h"

int main()
{
	torrent_t* _theTorrent = openTorrent("systemrescue-10.00-amd64.iso.torrent");

	//========DEBUG CODE BELOW==========
	int createdSocket = tcp_conn_createSocket();

	conpool_t* theConnectionPool = conpool_createPool();

	conc_queue* outgoingPacketQueue = conpool_createConnection(theConnectionPool,createdSocket,tcp_conn_outgoingFunction,tcp_conn_incomingFunction,tcp_conn_processingFunction);
	getchar();

	char buffer[50] = {0};
	sprintf(buffer, "%s\n", "Macaroane cu branza baaa");

	void* theBuiltPacket = tcp_conn_buildPacket(buffer,sizeof(buffer),0);
	conc_queue_push(outgoingPacketQueue,theBuiltPacket);
	getchar();

}
