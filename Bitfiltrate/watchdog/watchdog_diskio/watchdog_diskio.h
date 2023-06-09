/*
 * watchdog_diskio.h
 *
 *  Created on: Jun 8, 2023
 *      Author: kiraly
 */

#include "../../piecetracker/piecetracker.h"
#include "../watchdog.h"

typedef struct
{
	//=== API VARIABLES ===
	/*
	 * The piece tracker which keeps track of what pieces have been completed.
	 */
	piecetracker_t* thePieceTracker;
	//=== INTERNAL USE ===

	int theFileDescriptor;

} watchdog_diskio_t;

watchdog_diskio_t* watchdog_diskio_init(watchdog_t* __theWatchdog,piecetracker_t* __thePieceTracker);
