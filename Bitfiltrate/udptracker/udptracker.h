/*
 * udptracker.h
 *
 *  Created on: May 9, 2023
 *      Author: kiraly
 */

#ifndef UDPTRACKER_UDPTRACKER_H_
#define UDPTRACKER_UDPTRACKER_H_

#include <stdint.h>

typedef struct
{
	/*
	 * Sent by the tracker upon connection
	 */
	uint64_t connectionID;

	/*
	 * Randomly generated peer id
	 */
	uint8_t peerID[20];

	/*
	 * Statistics regarding the number of bytes since "started".
	 */
	uint64_t downloaded;
	uint64_t uploaded;
	uint64_t left;

} udptrack_t;



#endif /* UDPTRACKER_UDPTRACKER_H_ */
