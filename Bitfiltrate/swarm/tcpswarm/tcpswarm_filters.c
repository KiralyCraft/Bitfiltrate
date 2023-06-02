/*
 * tcpswarm_filters.c
 *
 *  Created on: Jun 2, 2023
 *      Author: kiraly
 */

#include "tcpswarm_filters.h"
#include "dlinkedlist.h"
#include "../peer/peer_networkdetails.h"
#include "tcppeer/tcppeer.h"

uint8_t tcpswarm_filters_filterByNetworkStatus(dlinkedlist_t* __thePeerList,swarm_filters_peerdata_t* __theFilteredPeers, void* __filterCriteria)
{
	peer_networkconfig_status_h _theDesiredNetworkStatus = *((peer_networkconfig_status_h*)__filterCriteria);

	size_t _currentPeerCount = dlinkedlist_getCount(__thePeerList);
	for (size_t _peerIterator = 0; _peerIterator < _currentPeerCount; _peerIterator++)
	{
		tcppeer_t* _thePeer = dlinkedlist_getPosition(_peerIterator, __thePeerList);
		if (_thePeer->peerNetworkConfig->peerConnectionStatus == _theDesiredNetworkStatus)
		{
			uint8_t _insertionSuccess = dlinkedlist_insertElement(_thePeer,__theFilteredPeers->peerData);
			if (_insertionSuccess == 0)
			{
				return 0;
			}
		}
	}
	return 1;
}
uint8_t tcpswarm_filters_filterByIncomingPacketTime(dlinkedlist_t* __thePeerList,swarm_filters_peerdata_t* __theFilteredPeers, void* __filterCriteria)
{
	return 0;
}
uint8_t tcpswarm_filters_filterByOutgoingPacketTime(dlinkedlist_t* __thePeerList,swarm_filters_peerdata_t* __theFilteredPeers, void* __filterCriteria)
{
	return 0;
}
uint8_t tcpswarm_filters_filterByNonzeroIncomingPacketCount(dlinkedlist_t* __thePeerList,swarm_filters_peerdata_t* __theFilteredPeers, void* __filterCriteria)
{
	uint8_t _intendedPacketID = *((uint8_t*)__filterCriteria);

	size_t _currentPeerCount = dlinkedlist_getCount(__thePeerList);
	for (size_t _peerIterator = 0; _peerIterator < _currentPeerCount; _peerIterator++)
	{
		tcppeer_t* _thePeer = dlinkedlist_getPosition(_peerIterator, __thePeerList);
		if (((_thePeer->packetsReceivedBitfield >> _intendedPacketID) & 0x1) == 1)
		{
			uint8_t _insertionSuccess = dlinkedlist_insertElement(_thePeer,__theFilteredPeers->peerData);
			if (_insertionSuccess == 0)
			{
				return 0;
			}
		}
	}
	return 1;
}
