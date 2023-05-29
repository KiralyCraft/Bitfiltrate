/*
 * peer_networkdetails.c
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#include "../peer/peer_networkdetails.h"

#include <stdint.h>
#include <stdlib.h>


peer_networkconfig_h* peer_networkdetails_generatePeerDetails(uint32_t __peerIP,uint16_t __peerPort)
{
	peer_networkconfig_h* _newPeerData = malloc(sizeof(peer_networkconfig_h));

	_newPeerData -> peerConnectionStatus = PEER_NEWBORN;
	_newPeerData -> peerIP = __peerIP;
	_newPeerData -> peerPort = __peerPort;
	_newPeerData -> peerSocket = 0; //This is just initialized, but it doesn't mean the socket is valid

	return _newPeerData;
}
