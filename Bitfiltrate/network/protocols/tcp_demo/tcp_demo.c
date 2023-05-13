/*
 * tcp_demo.c
 *
 *  Created on: May 11, 2023
 *      Author: kiraly
 */


#include "tcp_demo.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#define TCP_CHUNK_SIZE 1

int tcp_conn_createSocket()
{
	int sockfd = 0, n = 0;
	char recvBuff[1024];
	struct sockaddr_in serv_addr;


	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Error : Connect Failed \n");
	}

	return sockfd;
}

void* tcp_conn_buildPacket(void* __dataToSend, size_t __dataLength, size_t __dataOffset)
{
	tcp_conn_packet_t* _theBuiltPacket = malloc(sizeof(tcp_conn_packet_t));

	_theBuiltPacket -> packetData = __dataToSend;
	_theBuiltPacket -> packetDataOffset = __dataOffset;
	_theBuiltPacket -> packetLength = __dataLength;

	return _theBuiltPacket;
}

void tcp_conn_outgoingFunction(int __socketDescriptor, void* __outgoingData)
{
	tcp_conn_packet_t* _theOutgoingPacket = __outgoingData;

	uint8_t* _theActualOutgoingData = _theOutgoingPacket -> packetData;
	uint32_t _theActualOutgoingDataLength = _theOutgoingPacket -> packetLength;
	uint32_t _theActualOutoingDataPacketOffset = _theOutgoingPacket -> packetDataOffset;

	free(_theOutgoingPacket);

	uint8_t* _currentDataLocation = _theActualOutgoingData + _theActualOutoingDataPacketOffset;
	uint32_t _totalWrittenBytes = 0;

	while (_totalWrittenBytes < _theActualOutgoingDataLength)
	{
		uint32_t _chunkSize = _theActualOutgoingDataLength - _totalWrittenBytes; //Attempt to write everything at once

		if (_chunkSize > TCP_CHUNK_SIZE) //If it doesn't fir in our packet size constrictions
		{
			_chunkSize = TCP_CHUNK_SIZE; //Limit it to our imposed limit
		}

		uint32_t _bytesWritten = write(__socketDescriptor, _currentDataLocation + _totalWrittenBytes, _chunkSize); //Account for this thing being an error

		_totalWrittenBytes = _totalWrittenBytes + _bytesWritten;
	}

}
void* tcp_conn_incomingFunction(int __socketDescriptor)
{
	uint8_t* _expectedBuffer = malloc(TCP_CHUNK_SIZE);
	uint32_t _bytesRead = read(__socketDescriptor, _expectedBuffer, TCP_CHUNK_SIZE);

	tcp_conn_packet_t* _allocatedPacket = malloc(sizeof(tcp_conn_packet_t));
	_allocatedPacket->packetData = _expectedBuffer;
	_allocatedPacket->packetLength = _bytesRead;
	_allocatedPacket->packetDataOffset = 0;

	return _allocatedPacket;
}
void tcp_conn_processingFunction(void* __dataToProcess)
{
	tcp_conn_packet_t* _packetToProcess = __dataToProcess;

	uint8_t* _dataToProcess = _packetToProcess->packetData;
	uint32_t _dataLength = _packetToProcess->packetLength;
	uint32_t _dataOffset = _packetToProcess->packetDataOffset;

	free(_packetToProcess);
	printf("%.*s", _dataLength, _dataToProcess+_dataOffset);
	free(_dataToProcess);

}
