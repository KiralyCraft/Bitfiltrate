/*
 * swarm.h
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#ifndef SWARM_TCPSWARM_TCPSWARM_H_
#define SWARM_TCPSWARM_TCPSWARM_H_

#include "../swarm.h"

/*
 * This method creates a definition for a TCP Swarm.
 *
 * The returned method contains pointers to functions that process the different aspects of TCP Peer Communication.
 * This includes the incoming network communication function, the outgoing variant and the processing element.
 *
 * The definition should also include a post-processing function, which is where processed replies should go to once they have been processed by the definition.
 * It is to be assumed that the post-processing function should accept universal language processed packets.
 */
swarm_definition_t* tcpswarm_createDefinition(void (*__thePostProcessingFunction)(void*));

#endif /* SWARM_TCPSWARM_TCPSWARM_H_ */
