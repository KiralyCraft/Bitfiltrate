/*
 * udp.h
 *
 *  Created on: May 11, 2023
 *      Author: kiraly
 */

#ifndef NETWORK_UDP_DEMO_UDP_H_
#define NETWORK_UDP_DEMO_UDP_H_

#include <stdint.h>
#include <stddef.h>

typedef struct
{
	uint8_t* packetData;
	size_t packetLength;
	size_t packetDataOffset;
} udp_conn_packet_t;

int udp_conn_createSocket();

/*
 * Allocates and builds an implementation-specific packet that can be submitted to the worker queue.
 */
void* udp_conn_buildPacket(void* __dataToSend, size_t __dataLength, size_t __dataOffset);

/*
 * Networking outgoing function - UDP specific implementation.
 *
 * This function frees the encapsulated connection packet upon sending.
 */
void udp_conn_outgoingFunction(int __socketDescriptor, void* __outgoingData);
/*
 * Networking incoming function - UDP specific implementation.
 *
 * This function allocates both a buffer and the packet encapsulation, so the functions that benefit from this function
 * must make sure to clean up.
 */
void* udp_conn_incomingFunction(int __socketDescriptor);
/*
 * Network processing function for incoming messages - UDP specific implementation.
 *
 * This function receives a packet from the incoming function, and must make sure to clean up both the packet and the buffer, after finishing the processing.
 */
void udp_conn_processingFunction(void* __dataToProcess);

#endif /* NETWORK_UDP_DEMO_UDP_H_ */
