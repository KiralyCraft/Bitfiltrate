/*
 * watchdog.h
 *
 *  Created on: May 25, 2023
 *      Author: kiraly
 */

#ifndef WATCHDOG_WATCHDOG_H_
#define WATCHDOG_WATCHDOG_H_

#include "dlinkedlist.h"
#include <pthread.h>

typedef struct
{
	dlinkedlist_t* watchdogEntries;
} watchdog_t;

typedef struct
{
	pthread_t theThread;
	void* watchdogContext;
	void (*watchdogFunction)(void*);
} watchdog_entry_t;

/*
 * Creates a watchdog supervisor that handles arbitrary recurring tasks.
 */
watchdog_t* watchdog_createWatchdogSupervisor();

/*
 * Submits a watchdog for execution. This function requires:
 * - A watchdog context
 * - A watchdog execution function
 * - The watchdog monitor itself
 *
 * This function returns 0 if anything went wrong, or 1 otherwise.
 *
 * Submissions are NOT thread safe.
 */
uint8_t watchdog_submitWatchdog(void* __watchdogContext, void (*__watchdogFunction)(void*), watchdog_t* __theWatchdog);


#endif /* WATCHDOG_WATCHDOG_H_ */
