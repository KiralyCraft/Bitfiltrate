/*
 * peerfeed.h
 *
 *  Created on: Jun 9, 2023
 *      Author: kiraly
 */

#ifndef PEERFEED_PEERFEED_H_
#define PEERFEED_PEERFEED_H_

#include "../swarm/peer/peer_networkdetails.h"
#include "concurrent_queue.h"

/*
 * This method takes a file path, which is supposed to be in CSV format, containing lines of peers
 * separated by :. Out of these, only the first argument is taken into consideration, which is then split into an IP and a Port.
 *
 * If anything went wrong, this method returns 0. Otherwise, it returns 1.
 */
uint8_t peerfeed_ingestPeersFromFile(conc_queue_t* __peerIngestionQueue,char* __theFile);

#endif /* PEERFEED_PEERFEED_H_ */
