/*
 * conpool_executor.c
 *
 *  Created on: May 11, 2023
 *      Author: kiraly
 */

#include "conpool_executor.h"
#include "conpool.h"
#include "concurrent_queue.h"

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
		void* _receivedData = _connectionDetails->incomingFunction(_connectionDetails->socketDescription);
		_connectionDetails->processingFunction(_receivedData);
	}

	return NULL;
}
