/*
 * udptracker_proto.h
 *
 *  Created on: May 20, 2023
 *      Author: kiraly
 */

#ifndef UDPTRACKER_UDPTRACKER_PROTO_H_
#define UDPTRACKER_UDPTRACKER_PROTO_H_

#include "udptracker_comm.h"

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
	UDP_TRACKER_PACKET_SCRAPE,
	UDP_TRACKER_PACKET_ERROR
} udptrack_packet_type_t;

/*
 * A generic wrapper that stores all common features of a reply packet from the tracker.
 *
 * Based on the packet
 */
typedef struct
{
	udptrack_packet_type_t packetType;
	int32_t packetTransactionID;
	void* packetData;
} udptrack_packet_reply_t;

//=== CONNECT REPLY PACKET ===
typedef struct
{
	int64_t connectionID;
} udptrack_packet_reply_data_connect_t;

//=== ANNOUNCE REPLY PACKET ===
typedef struct
{
	int32_t peerIP;
	uint16_t peerPort;
} udptrack_packet_reply_data_announce_peer_t;

typedef struct
{
	int32_t reannounceInterval;
	int32_t leechCount;
	int32_t seedCount;

	uint32_t peerCount;
	udptrack_packet_reply_data_announce_peer_t* thePeers;
} udptrack_packet_reply_data_announce_t;

typedef struct
{
	uint32_t errorMessageLength;
	uint8_t* errorMessage;
} udptrack_packet_reply_data_error_t;

//=== SCRAPE REPLY PACKET ===
typedef struct
{
	int32_t completeCount;
	int32_t downloadCount;
	int32_t incompleteCount;
} udptrack_packet_reply_data_scrape_t;

/*
 * Helper function that generates transaction IDs.
 */
int32_t udptracker_proto_generateTransactionID();

/*
 * Build a connection packet that is ready to be sent out to the tracker.
 */
udptrack_packet_t* udptracker_proto_requestConnectPacket(int32_t __transactionID);

/*
 * A generic function that is intended to process received data from a socket.
 *
 * This function runs in a different thread, handled by the connection pool, and is guaranteed to be
 * different than the thread calling it.
 *
 * The return argument may or may not be used, do not rely on it for anything.
 */
void* udptracker_proto_processRawGenericPacket(void* __dataBundle);

/*
 * Destroys a reply packet.
 */
void udptracker_proto_destroyReplyPacket(udptrack_packet_reply_t* __thePacket);

/*
 * Destroys a generic packet.
 *
 * This can either be an outgoing packet, or a packet that is contained within another reply packet, as the packet data itself.
 */
void udptracker_proto_destroyPacket(udptrack_packet_t* __thePacket);

#endif /* UDPTRACKER_UDPTRACKER_PROTO_H_ */
