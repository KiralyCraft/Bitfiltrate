/*
 * swarm.c
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#include "swarm.h"
#include "../torrentinfo/torrentinfo.h"
#include "../network/conpool.h"

#include <stdlib.h>
#include <string.h>

swarm_t* swarm_createPeerSwarm(swarm_definition_t* __theSwarmDefinition,torrent_t* __theTorrentInfo,conpool_t* __connectionPool)
{
	swarm_t* _newSwarm = malloc(sizeof(swarm_t));
	memcpy(_newSwarm->torrentHash,__theTorrentInfo->torrentHash,sizeof(uint8_t) * 20);

	_newSwarm->currentSwarmDefinition = __theSwarmDefinition;
	_newSwarm->currentPeerData = dlinkedlist_createList();
	aici am ramas, pentru create swarm cu connection pool si toate cele; connection pool vine din argumente
	cand se face ingest la un peer, submit a new connection in pool care are un income function comun (tip network)
	si un processing function de undeva din "protocol", care apoi aduce rezulatele aici dupa procesare, asemanator cu udp tracker.

	la inceput, cand se face create la peer swarm, ar trebui sa se construiascsa si un anume bitfield sau infromatiile neceare.
	acum ca vad, asta e de fapt echivalentul al udptracker, deci aici se face procesarea de la ce o trimis protocolul. astea de aici apoi dau
	(sau tin) cumva la watchdog, ca sa scrie fisierele pe disk.

	oare sa nu scrie de fapt astea pe disk cumva? direct de aici? vedem.
}
uint8_t swarm_ingestPeer(swarm_t* __theSwarm, peer_networkconfig_h* __peerDetails)
{
	swarm_definition_t* theSwarmDefinition = __theSwarm->currentSwarmDefinition;
	//add the thing result of the function process to the list
}
