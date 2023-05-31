/*
 * tcpswarm_comm.h
 *
 *  Created on: May 29, 2023
 *      Author: kiraly
 */

#ifndef SWARM_TCPSWARM_TCPSWARM_COMM_H_
#define SWARM_TCPSWARM_TCPSWARM_COMM_H_


#include "tcppeer/tcppeer.h"
#include "../swarm.h"
#include "../peer/peer_networkdetails.h"
#include "../../network/conpool.h"

/*
 * A data type that encapsulates the data to be sent (or to be received) over the network.
 */
typedef struct
{
	uint8_t* packetData;
	size_t packetSize;
} tcpswarm_packet_t;

/*
 * This function initializes a connection with a peer, based solely on the given argument and the fields contained inside.
 *
 * It's general purpose is to establish a connection towards the provided IP and port, to set the socket descriptor and to update the connection status accordingly.
 *
 * This method returns 0 on failure, or 1 otherwise.
 */
uint8_t _tcpswarm_peerInitialize(tcppeer_t* __thePeerNetworkConfiguration);
void* _tcpswarm_peerIngestFunction(peer_networkconfig_h* __thePeerConnectionDetails,swarm_definition_t* __theSwarmDefinition, conpool_t* __theConnectionPool);

/*
 * The outgoing function receives a bundle of data as the socket descriptor, and a tcpswarm_packet_t for the outgoing data.
 * The optional argument and the actual data received for the socket descriptor is up to the connection pool to provide, based on what it was served.
 */
void _tcpswarm_outgoingFunction(void* __socketDescriptor, void* __outgoingData, void* __optionalArgument);
void* _tcpswarm_incomingFunction(void* __socketDescriptor, void* __optionalArgument);
void* _tcpswarm_processingFunction(void* __rawGenericPacket);

#endif /* SWARM_TCPSWARM_TCPSWARM_COMM_H_ */
