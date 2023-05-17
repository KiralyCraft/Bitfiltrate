/*
 * udp_demo.c
 *
 *  Created on: May 11, 2023
 *      Author: kiraly
 */


#include "udp_demo.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#define UDP_CHUNK_SIZE 1500

int udp_conn_createSocket()
{
	int sockfd = 0, n = 0;
	char recvBuff[1024];
	struct sockaddr_in serv_addr;


	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		printf("\n Error : Could not create socket \n");
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5000);

	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	{
		printf("\n inet_pton error occured\n");
	}

	return sockfd;
}

void* udp_conn_buildPacket(void* __dataToSend, size_t __dataLength, size_t __dataOffset)
{
	udp_conn_packet_t* _theBuiltPacket = malloc(sizeof(udp_conn_packet_t));

	_theBuiltPacket -> packetData = __dataToSend;
	_theBuiltPacket -> packetDataOffset = __dataOffset;
	_theBuiltPacket -> packetLength = __dataLength;

	return _theBuiltPacket;
}

void udp_conn_outgoingFunction(int __socketDescriptor, void* __outgoingData)
{
	udp_conn_packet_t* _theOutgoingPacket = __outgoingData;

	uint8_t* _theActualOutgoingData = _theOutgoingPacket -> packetData;
	uint32_t _theActualOutgoingDataLength = _theOutgoingPacket -> packetLength;
	uint32_t _theActualOutoingDataPacketOffset = _theOutgoingPacket -> packetDataOffset;

	free(_theOutgoingPacket);

	//DEBUG MOVE THESE

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5000);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	uint8_t* _currentDataLocation = _theActualOutgoingData + _theActualOutoingDataPacketOffset;

	ssize_t _bytesSent = sendto(__socketDescriptor, _currentDataLocation, _theActualOutgoingDataLength , 0 , (const struct sockaddr *) &servaddr, sizeof(servaddr));

	printf("Sent %d\n",_bytesSent);

}
void* udp_conn_incomingFunction(int __socketDescriptor)
{
	uint8_t* _expectedBuffer = malloc(UDP_CHUNK_SIZE);
//	uint32_t _bytesRead = read(__socketDescriptor, _expectedBuffer, UDP_CHUNK_SIZE);

	printf("Listening\n");
	struct sockaddr_in cliaddr;
	socklen_t len;
	ssize_t _bytesRead = recvfrom(__socketDescriptor, _expectedBuffer, UDP_CHUNK_SIZE, MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len);

	printf("Received %d\n",_bytesRead);
	udp_conn_packet_t* _allocatedPacket = malloc(sizeof(udp_conn_packet_t));
	_allocatedPacket->packetData = _expectedBuffer;
	_allocatedPacket->packetLength = _bytesRead;
	_allocatedPacket->packetDataOffset = 0;

	return _allocatedPacket;
}
void udp_conn_processingFunction(void* __dataToProcess)
{
	udp_conn_packet_t* _packetToProcess = __dataToProcess;

	uint8_t* _dataToProcess = _packetToProcess->packetData;
	uint32_t _dataLength = _packetToProcess->packetLength;
	uint32_t _dataOffset = _packetToProcess->packetDataOffset;

	free(_packetToProcess);
	printf("%.*s", _dataLength, _dataToProcess+_dataOffset);
	free(_dataToProcess);

}
