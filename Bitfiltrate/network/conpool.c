/*
 * conpool.c
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#include "conpool.h"
#include "concurrent_queue.h"
#include "conpool_executor.h"
#include <stdlib.h>

conpool_t* conpool_createPool()
{
	conpool_t* _createdPool = malloc(sizeof(conpool_t));

	_createdPool->activeConnectionThreads = dlinkedlist_createList();

	return _createdPool;
}

/*
 * Helper function for submitting a connection to the connection pool.
 * This function handles the creation of threads and things alike, as well.
 */
uint8_t _conpool_submitConnection(conpool_t* __theConnectionPool, conpool_connection_details_t* __theConnectionDetails)
{
	conpool_connection_t* _newConnection = malloc(sizeof(conpool_connection_t));
	_newConnection -> connectionDetails = __theConnectionDetails;

	uint32_t _sendingThreadCreationResult = pthread_create(&_newConnection->associatedSendingThread, NULL, conpool_executorSendingFunction, (void*) __theConnectionDetails);
	uint32_t _receivingThreadCreationResult = pthread_create(&_newConnection->associatedReceivingThread, NULL, conpool_executorReceivingFunction, (void*) __theConnectionDetails);
	if (_sendingThreadCreationResult == 0 && _receivingThreadCreationResult == 0)
	{
		return 1; //Success
	}
	return 0;
}

conc_queue* conpool_createConnection(conpool_t* __theConnectionPool, int __socketDescription,void (*__outgoingFunction)(int,void*),void* (*__incomingFunction)(int),void (*__processingFunction)(void*),void* __optionalArgument)
{
	conpool_connection_details_t* _theNewConnection = malloc(sizeof(conpool_connection_details_t));

	_theNewConnection -> socketDescription = __socketDescription;

	_theNewConnection -> optionalArgument = __optionalArgument;

	_theNewConnection -> outgoingFunction = __outgoingFunction;
	_theNewConnection -> incomingFunction = __incomingFunction;
	_theNewConnection -> processingFunction = __processingFunction;

	conc_queue_init(&_theNewConnection -> outgoingPacketQueue);

	uint8_t _submissionResult = _conpool_submitConnection(__theConnectionPool, _theNewConnection);

	if (_submissionResult == 0)
	{
		conc_queue_destroy(_theNewConnection -> outgoingPacketQueue);
		free(_theNewConnection);
		return NULL; //Something went wrong submitting this connection
	}
	else
	{
		return _theNewConnection -> outgoingPacketQueue;
	}
}

uint8_t conpool_destroyPool(conpool_t* __thePool)
{
	//TODO actually close down the connections within the pool, destroy them and shut them down
	dlinkedlist_destroy(__thePool -> activeConnectionThreads);
	free(__thePool);

	return 1;
}
