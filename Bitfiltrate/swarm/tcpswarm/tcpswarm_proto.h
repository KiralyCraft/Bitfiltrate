/*
 * tcpswarm_proto.h
 *
 *  Created on: May 29, 2023
 *      Author: kiraly
 */

#ifndef SWARM_TCPSWARM_TCPSWARM_PROTO_H_
#define SWARM_TCPSWARM_TCPSWARM_PROTO_H_

#include "../swarm_message_types.h"
#include <stdint.h>



/*
 * The processing function which gets called with neatly wrapped packages from the network.
 * This function should clear the data that it receives, such as the packet data (but of course not all arguments)
 */
void* _tcpswarm_processingFunction(void* __rawGenericPacket);

/*
 * This function generates packets based on the given message type, and expected corresponding data given by means
 * of the second, optional data parameter.
 */
void* _tcpswarm_generatePacket(swarm_message_e __theMessageType, void* __optionalData);

/*
 * This is a protocol-specific function, that if given a packet (of presumably compatible type)
 * it adds it in the outgoing queue for this peer, such that it is is eventually transmitted.
 */
uint8_t _tcpswarm_peerQueueOutgoingPacket(void* __thePacket,void* __thePeerData);


#endif /* SWARM_TCPSWARM_TCPSWARM_PROTO_H_ */
