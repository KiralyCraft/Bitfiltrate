/*
 * watchdog.c
 *
 *  Created on: May 25, 2023
 *      Author: kiraly
 */

#include "watchdog.h"
#include "dlinkedlist.h"
#include <stdlib.h>

void* _watchdog_executorFunction(void* __theArguments);

watchdog_t* watchdog_createWatchdogSupervisor()
{
	watchdog_t* _theWatchdog = malloc(sizeof(watchdog_t));
	_theWatchdog->watchdogEntries = dlinkedlist_createList();
	return _theWatchdog;
}

uint8_t watchdog_submitWatchdog(void* __watchdogContext, void (*__watchdogFunction)(void*), watchdog_t* __theWatchdog)
{
	dlinkedlist_t* _watchdogEntries = __theWatchdog->watchdogEntries;

	watchdog_entry_t* _theWatchdogEntry = malloc(sizeof(watchdog_entry_t));
	_theWatchdogEntry->watchdogContext = __watchdogContext;
	_theWatchdogEntry->watchdogFunction = __watchdogFunction;
	uint32_t _watchdogSubmissionResult = pthread_create(&_theWatchdogEntry->theThread,NULL,_watchdog_executorFunction,_theWatchdogEntry);

	if (_watchdogSubmissionResult == 0)
	{
		uint8_t _watchdogStoreResult = dlinkedlist_insertElement(_theWatchdogEntry,_watchdogEntries);
		if (_watchdogStoreResult == 1)
		{
			return 1;
		}
	}
	else
	{
		free(_theWatchdogEntry);
	}

	return 0;
}

/*
 * This is a helper function which actually executes the watchdog in a loop, according to the provided context.
 *
 * Despite this function being given the whole context, it must not touch the thread reference itself.
 */
void* _watchdog_executorFunction(void* __theArguments)
{
	watchdog_entry_t* _theWatchdogEntry = __theArguments;
	while(1) //TODO define actual stop condition for watchdogs, maybe add a flag for the status?
	{
		_theWatchdogEntry->watchdogFunction(_theWatchdogEntry->watchdogContext);
	}
	return NULL;
}
