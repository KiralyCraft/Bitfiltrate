/*
 * tcpswarm_comm.c
 *
 *  Created on: May 29, 2023
 *      Author: kiraly
 */

#include "tcpswarm_comm.h"
#include "../swarm.h"
#include "../peer/peer_networkdetails.h"
#include "tcppeer/tcppeer.h"
#include "../../network/conpool.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

#include <endian.h>

#include <string.h>

#define TCP_CHUNK_SIZE 1500
#define TCP_CONNECTION_TIMEOUT 10

uint8_t _tcpswarm_peerInitialize(tcppeer_t* __thePeerConfiguration)
{
	//Ever since we've moved eager initialization to it's lazy counterpart, this method doesn't really do anything now.
	pthread_mutex_init(&(__thePeerConfiguration->syncMutex),NULL);
	pthread_cond_init(&(__thePeerConfiguration->syncCondvar),NULL);

	return 1;
}
/*
 * This is a helper function, it should NEVER be called externally.
 *
 * This method initializes a peer connection based on the details it has been given.
 * It also attempts to connect, so assume that this method BLOCKS.
 *
 * This method returns 0 if anything went wrong, or 1 otherwise.
 */
uint8_t _tcpswarm_peerConnect(peer_networkconfig_h* __thePeerNetworkConfiguration)
{
	int _socketDescriptor = 0;
	struct sockaddr_in _theSocketAddress;
	if ((_socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return 0;
	}

    //=== ACTUAL CONNECTION ===
	memset(&_theSocketAddress, 0, sizeof(_theSocketAddress));

	_theSocketAddress.sin_family = AF_INET;
	_theSocketAddress.sin_addr.s_addr = __thePeerNetworkConfiguration->peerIP; //Hind to any available local IP (but not IPv6, as indicated by the previous line)
	_theSocketAddress.sin_port = htobe16(__thePeerNetworkConfiguration->peerPort);

	__thePeerNetworkConfiguration -> peerConnectionStatus = PEER_PENDING;

	int _connectionResult = connect(_socketDescriptor, (struct sockaddr*) &_theSocketAddress, sizeof(_theSocketAddress));
	if (_connectionResult < 0)
	{
		printf("Aww failed with %s %d in _tcpswarm_peerConnect\n",inet_ntoa(_theSocketAddress.sin_addr),__thePeerNetworkConfiguration->peerPort);
		__thePeerNetworkConfiguration -> peerConnectionStatus = PEER_ERROR;
		return 0;
	}
	else
	{
		printf("Eyy connected with %s %d in _tcpswarm_peerConnect\n",inet_ntoa(_theSocketAddress.sin_addr),__thePeerNetworkConfiguration->peerPort);
		__thePeerNetworkConfiguration -> peerSocket = _socketDescriptor;
		__thePeerNetworkConfiguration -> peerConnectionStatus = PEER_INITIALIZED;
		return 1;
	}
}

void _tcpswarm_outgoingFunction(void* __socketDescriptor, void* __outgoingData, void* __optionalArgument)
{
	printf("Sent some data!\n");
	//TODO make sure the packet is freed after it has been sent.

	tcppeer_t* _thePeerDetails = __socketDescriptor;
	peer_networkconfig_h* _thePeerNetworkConfiguration = _thePeerDetails -> peerNetworkConfig;

	if (_thePeerNetworkConfiguration->peerConnectionStatus == PEER_ERROR)
	{
		return; //If the peer is currently an error, do not bother to send data to it anymore.
	}

	if (_thePeerNetworkConfiguration->peerConnectionStatus == PEER_NEWBORN) //If we are attempting to send data to a peer, but no actual connection has been established yet
	{
		pthread_mutex_lock(&(_thePeerDetails->syncMutex));
		uint8_t _connectionResult = _tcpswarm_peerConnect(_thePeerNetworkConfiguration);
		pthread_cond_signal(&(_thePeerDetails->syncCondvar));
		pthread_mutex_unlock(&(_thePeerDetails->syncMutex));
		if (_connectionResult == 0)
		{
			printf("Connection failed :(\n");
			return; //Do not attempt to send data to this peer, as it has indicated connection has failed
		}
	}


	tcpswarm_packet_t* _theOutgoingPacket = __outgoingData;

	int _socketDescriptor = _thePeerNetworkConfiguration->peerSocket;
	uint8_t* _theActualOutgoingData = _theOutgoingPacket -> packetData;
	size_t _theActualOutgoingDataLength = _theOutgoingPacket -> packetSize;


	uint8_t* _currentDataLocation = _theActualOutgoingData;
	uint32_t _totalWrittenBytes = 0;

	while (_totalWrittenBytes < _theActualOutgoingDataLength)
	{
		uint32_t _chunkSize = _theActualOutgoingDataLength - _totalWrittenBytes; //Attempt to write everything at once

		if (_chunkSize > TCP_CHUNK_SIZE) //If it doesn't fir in our packet size constrictions
		{
			_chunkSize = TCP_CHUNK_SIZE; //Limit it to our imposed limit
		}

		uint32_t _bytesWritten = write(_socketDescriptor, _currentDataLocation + _totalWrittenBytes, _chunkSize); //Account for this thing being an error

		_totalWrittenBytes = _totalWrittenBytes + _bytesWritten;
	}
	free(_theActualOutgoingData);
	free(_theOutgoingPacket);
}
void* _tcpswarm_incomingFunction(void* __socketDescriptor, void* __optionalArgument)
{
	tcppeer_t* _thePeerDetails = __socketDescriptor;
	peer_networkconfig_h* _thePeerNetworkConfiguration = _thePeerDetails->peerNetworkConfig;

	//=== WAITING FOR CONNECTION TO BE UP ===
	if (_thePeerNetworkConfiguration->peerConnectionStatus == PEER_NEWBORN)
	{
		pthread_mutex_lock(&(_thePeerDetails->syncMutex));
		pthread_cond_wait(&(_thePeerDetails->syncCondvar),&(_thePeerDetails->syncMutex));
		pthread_mutex_unlock(&(_thePeerDetails->syncMutex)); //Hopefully, the peer connection status should've changed by now.
	}
	//=== PROCESSING ACTUAL DATA ===
	if (_thePeerNetworkConfiguration->peerConnectionStatus == PEER_INITIALIZED) //The peer did not handshake yet
	{

		uint8_t _handshakeReply[68];
		uint32_t _bytesRead = read(_thePeerNetworkConfiguration->peerSocket, _handshakeReply, 68);

//		printf("Got some data!\n");
		if (_bytesRead != 68)
		{
			; //TODO handle situation where not enough bytes have been read
		}
		else
		{
//			printf("Yay got handsheic Wireshark me!\n");
		}
	}
	else if (_thePeerNetworkConfiguration->peerConnectionStatus == PEER_CONNECTED) //The peer has completed the handshake, all further messages are prefixed by length
	{
//		uint8_t _incomingMessageLength[4];
		//TODO receive 4 byte length prefix, message ID and so on
	}

	return NULL;
}

