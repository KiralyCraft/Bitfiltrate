/*
 * udptracker_comm.h
 *
 *  Created on: May 20, 2023
 *      Author: kiraly
 */

#include <stdint.h>
#ifndef UDPTRACKER_COMM_H_
#define UDPTRACKER_COMM_H_
/*
 * An enumeration that keeps track of packet types when communicating with
 * a tracker.
 *
 * It is designed for use in user-defined types.
 */
typedef enum
{
	UDP_TRACKER_PACKET_CONNECT,
	UDP_TRACKER_PACKET_ANNOUNCE,
	UDP_TRACKER_PACKET_SCRAPE
} udptrack_packet_type_t;

/*
 * A data type that encapsulates the data to be sent to (or received from) a tracker.
 */
typedef struct
{
	uint8_t* packetData;
	uint32_t packetSize;
} udptrack_packet_t;

/*
 * The state of a conversation with an UDP tracker. It can be:
 *	UDP_TRACKER_CONV_PENDING - The conversation is set to start at some point
 *	UDP_TRACKER_CONV_STARTED - The conversation has been started, and is waiting for a response
 *	UDP_TRACKER_CONV_FINISHED - The conversation has been replied to, and is considered finished
 *	UDP_TRACKER_CONV_TIMEOUT - The conversation timed out while waiting for a reply
 */
typedef enum
{
	UDP_TRACKER_CONV_PENDING,
	UDP_TRACKER_CONV_STARTED,
	UDP_TRACKER_CONV_FINISHED,
	UDP_TRACKER_CONV_TIMEOUT
} udptrack_conversation_status_t;

/*
 * A data structure that keeps track of conversations with a tracker.
 */
typedef struct
{
	udptrack_conversation_status_t converstationStatus;
	udptrack_packet_type_t conversationType;
	udptrack_packet_t* outgoingRequest;
	udptrack_packet_t* incomingRequest;
} udptracker_conversation_t;

/*
 * Networking outgoing function - UDP specific implementation.
 */
void udptracker_comm_outgoingFunction(int __socketDescriptor, void* __outgoingData);
/*
 * Networking incoming function - UDP specific implementation.
 */
void* udptracker_comm_incomingFunction(int __socketDescriptor);
/*
 * Network processing function for incoming messages - UDP specific implementation.
 *
 * This function receives a packet from the incoming function, and must make sure to clean up both the packet and the buffer, after finishing the processing.
 */
void udptracker_comm_processingFunction(void* __dataToProcess);

#endif /* UDPTRACKER_COMM_H_ */
