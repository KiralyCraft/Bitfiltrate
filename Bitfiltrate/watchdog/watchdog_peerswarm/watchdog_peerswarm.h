/*
 * watchdog_peerswarm.h
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#ifndef WATCHDOG_WATCHDOG_PEERSWARM_WATCHDOG_PEERSWARM_H_
#define WATCHDOG_WATCHDOG_PEERSWARM_WATCHDOG_PEERSWARM_H_

#include "../watchdog.h"
#include "../../network/conpool.h"
#include "../../swarm/peer/peer_networkdetails.h"
#include "../../torrentinfo/torrentinfo.h"
#include "../../swarm/swarm.h"
#include "concurrent_queue.h"
#include <time.h>

typedef enum
{
	SWARM_EXEC_NORMAL,
	SWARM_EXEC_GUESS_PIECE_SIZE,
	SWARM_EXEC_CONFIRM_PIECE_SIZE
} watchdog_peerswarm_execution_mode_e;

//typedef enum
//{
//	time_t
//} watchdog_peerswarm_guessdata_t;
typedef struct
{
	//==== API VARIABLES ===
	/*
	 * This is the peer ingestion queue
	 */
	conc_queue* peerIngestionQueue;

	//==== INTERNAL USE ===
	/*
	 * This is the current generic swarm that this watchdog operates on.
	 */
	swarm_t* thePeerSwarm;

	/*
	 * This is the current swarm execution mode.
	 * Depending on this mode,
	 */
	watchdog_peerswarm_execution_mode_e swamExecutionMode;

	/*
	 * This is the piece size that this swarm uses, expressed as a logarithm of two.
	 */
	uint8_t swarmPieceSize;

	//=== PROTOCOL HACKS ===
	time_t timeGuessedPiece;

} watchdog_peerswarm_t;

watchdog_peerswarm_t* watchdog_peerswarm_init(watchdog_t* __theWatchdog,torrent_t* __torrentHash,conpool_t* __theConnectionPool);

/*
 * This is a function which should accept the given peer and add it to the watchdog (and therefore the pool).
 *
 * This function will always be executed asynchronously with everything else.
 * TODO: Currently this method doesn't differentiate between torrents submitted, and it assumes they are all the same (effectively ignoring the torrentdata)
 */
void watchdog_peerswarm_ingestPeer(watchdog_peerswarm_t* __thePeerSwarmWatchdog, peer_networkconfig_h* __peerConfig,torrent_t* __torrentData);


#endif /* WATCHDOG_WATCHDOG_PEERSWARM_WATCHDOG_PEERSWARM_H_ */
