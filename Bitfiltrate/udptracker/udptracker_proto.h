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
 * Build a connection packet that is ready to be sent out to the tracker.
 */
udptrack_packet_t* udptracker_proto_requestConnectPacket();
/*
 * Processes a connection reply packet.
 */
void udptracker_proto_receiveConnectPacket(udptrack_packet_t* __receivedPacket);

#endif /* UDPTRACKER_UDPTRACKER_PROTO_H_ */
