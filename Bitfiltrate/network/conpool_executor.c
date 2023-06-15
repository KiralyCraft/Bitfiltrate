/*
 * conpool_executor.c
 *
 *  Created on: May 11, 2023
 *      Author: kiraly
 */

#include "conpool_executor.h"
#include "conpool.h"
#include "concurrent_queue.h"
#include <stdlib.h>
#include <stdio.h>

void* conpool_executorSendingFunction(void* __providedData)
{
	conpool_connection_details_t* _connectionDetails = __providedData;

	//TODO implement a proper stopping technique for clean shutdowns
	//TODO implement error checking for the outgoing function. If anything happens, kill both this and the incoming function
	while(1)
	{
		if (_connectionDetails->connectionHealth == CONPOOL_CONNECTION_STATUS_DEAD)
		{
			break;
		}

		void* _poppedData = conc_queue_pop(_connectionDetails->outgoingPacketQueue); //This should be a blocking operation
		_connectionDetails->outgoingFunction(_connectionDetails->socketDescription,_poppedData,_connectionDetails->optionalArgument);
	}

	return NULL;
}
size_t alive = 0;
void* conpool_executorReceivingFunction(void* __providedData)
{
	conpool_connection_details_t* _connectionDetails = __providedData;
	alive++;
	//TODO implement a proper stopping technique for clean shutdowns
	while(1)
	{
		//==== RECEIVE DATA FROM THE CONNECTION ====
		void* _receivedData = _connectionDetails->incomingFunction(_connectionDetails->socketDescription,_connectionDetails->optionalArgument);
		if (_receivedData == NULL)
		{
			alive--;
//			printf("Connection died. Alive: %lu\n",alive);
			_connectionDetails->connectionHealth = CONPOOL_CONNECTION_STATUS_DEAD;
			break;
		}
		//==== PREPARE PASSING OF DATA BUNDLES ====
		void** _dataBundle = malloc(sizeof(void*)*2);
		_dataBundle[0] = _receivedData;
		_dataBundle[1] = _connectionDetails->executionContext;
		_dataBundle[2] = _connectionDetails->optionalArgument;
		//==== LAUNCH BACKGROUND EXECUTION OF DATA ====
		pthread_t _processingThread;
		pthread_attr_t _threadAttributes;
		pthread_attr_init(&_threadAttributes);
		pthread_attr_setdetachstate(&_threadAttributes, PTHREAD_CREATE_DETACHED);

		int _processingCreationResult = pthread_create(&_processingThread,&_threadAttributes,_connectionDetails->processingFunction,_dataBundle);
		if (_processingCreationResult != 0)
		{
			printf("DEBUG: Executor failed to launch processing thread for read packet!\n");
			; //TODO handle processing error
		}
	}

	return NULL;
}
