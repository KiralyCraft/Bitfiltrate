/*
 * swarm_filters.c
 *
 *  Created on: Jun 2, 2023
 *      Author: kiraly
 */

#include "swarm_filters.h"
#include "dlinkedlist.h"

#include <stdlib.h>

swarm_filters_peerdata_t* swarm_filters_createPeerFilterBucket()
{
	swarm_filters_peerdata_t* _theFilteredPeers = malloc(sizeof(swarm_filters_peerdata_t));
	_theFilteredPeers->peerData = dlinkedlist_createList();
	return _theFilteredPeers;
}

uint8_t swarm_filters_destroyPeerFilterBucket(swarm_filters_peerdata_t* __theFilteredBucket)
{
	uint8_t _bucketDestrctionResult = dlinkedlist_destroy(__theFilteredBucket->peerData);
	if (_bucketDestrctionResult == 0)
	{
		return 0;
	}
	free(__theFilteredBucket);
	return 1;
}
