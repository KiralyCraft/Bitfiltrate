/*
 * udptracker_comm.c
 *
 *  Created on: May 20, 2023
 *      Author: kiraly
 */
#include <stdlib.h>

#include "udptracker_comm.h"
#include "udptracker.h"

    #include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>


#define UDP_CHUNK_SIZE 1500


void udptracker_comm_outgoingFunction(void* __socketDescriptor, void* __outgoingData,void* __optionalArgument)
{
	//TODO what if the bytes we want to send are bigger than the max udp chunk size?
	udptrack_packet_t* _thePacket = __outgoingData;
	udptrack_networkconfig* _trackerNetworkConfiguration = __socketDescriptor;

	/*
	 * The outgoing and incoming requests are freed only once the conversation is deemed to be finished, or after returning an error, for debugging purposes.
	 */

	struct sockaddr_in _theTargetServer;
	_theTargetServer.sin_family = AF_INET;
	_theTargetServer.sin_port = htons(_trackerNetworkConfiguration->trackerPort);
	_theTargetServer.sin_addr.s_addr = _trackerNetworkConfiguration->trackerIP;

	ssize_t _bytesSent = sendto(_trackerNetworkConfiguration->trackerSocket, _thePacket->packetData, _thePacket->packetSize , 0 , (const struct sockaddr *) &_theTargetServer, sizeof(_theTargetServer));

	printf("%s\n",inet_ntoa(_theTargetServer.sin_addr));
	if (_bytesSent != _thePacket->packetSize)
	{
		//TODO error
		printf("Something went wrong uhoh\n");
	}
	else
	{
		printf("DEBUG: Request sent :)\n");
	}
}

void* udptracker_comm_incomingFunction(void* __socketDescriptor,void* __optionalArgument)
{
	udptrack_networkconfig* _trackerNetworkConfiguration = __socketDescriptor;
	uint8_t* _expectedBuffer = malloc(UDP_CHUNK_SIZE);

	socklen_t _receivedDataLength;
	ssize_t _bytesRead = recvfrom(_trackerNetworkConfiguration->trackerSocket, _expectedBuffer, UDP_CHUNK_SIZE, MSG_WAITALL,NULL,&_receivedDataLength);
	udptrack_packet_t* _receivedPacket = malloc(sizeof(udptrack_packet_t));
	_receivedPacket -> packetData = _expectedBuffer;
	_receivedPacket -> packetSize = _bytesRead;
	return _receivedPacket;
}
