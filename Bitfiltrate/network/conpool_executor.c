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

void* conpool_executorSendingFunction(void* __providedData)
{
	conpool_connection_details_t* _connectionDetails = __providedData;

	//TODO implement a proper stopping technique for clean shutdowns
	while(1)
	{
		void* _poppedData = conc_queue_pop(_connectionDetails->outgoingPacketQueue); //This should be a blocking operation
		_connectionDetails->outgoingFunction(_connectionDetails->socketDescription,_poppedData,_connectionDetails->optionalArgument);
	}

	return NULL;
}

void* conpool_executorReceivingFunction(void* __providedData)
{
	conpool_connection_details_t* _connectionDetails = __providedData;

	//TODO implement a proper stopping technique for clean shutdowns
	while(1)
	{
		//==== RECEIVE DATA FROM THE CONNECTION ====
		void* _receivedData = _connectionDetails->incomingFunction(_connectionDetails->socketDescription,_connectionDetails->optionalArgument);
		//==== PREPARE PASSING OF DATA BUNDLES ====
		void** _dataBundle = malloc(sizeof(void*)*2);
		_dataBundle[0] = _receivedData;
		_dataBundle[1] = _connectionDetails->executionContext;
		_dataBundle[2] = _connectionDetails->optionalArgument;
		//==== LAUNCH BACKGROUND EXECUTION OF DATA ====
		pthread_t _processingThread;
		int _processingCreationResult = pthread_create(&_processingThread,NULL,_connectionDetails->processingFunction,_dataBundle);
		if (_processingCreationResult != 0)
		{
			; //TODO handle processing error
		}
	}

	return NULL;
}
