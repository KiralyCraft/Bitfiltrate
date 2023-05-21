/*
 * udptracker_comm.h
 *
 *  Created on: May 20, 2023
 *      Author: kiraly
 */

#include <stdint.h>
#include <stddef.h>
#ifndef UDPTRACKER_COMM_H_
#define UDPTRACKER_COMM_H_

/*
 * A data type that encapsulates the data to be sent to (or received from) a tracker.
 */
typedef struct
{
	uint8_t* packetData;
	size_t packetSize;
} udptrack_packet_t;

/*
 * Networking outgoing function - UDP specific implementation.
 */
void udptracker_comm_outgoingFunction(void* __socketDescriptor, void* __outgoingData, void* __optionalArgument);
/*
 * Networking incoming function - UDP specific implementation.
 */
void* udptracker_comm_incomingFunction(void* __socketDescriptor, void* __optionalArgument);


#endif /* UDPTRACKER_COMM_H_ */
