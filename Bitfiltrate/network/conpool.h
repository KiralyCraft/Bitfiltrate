/*
 * conpool.h
 *
 *  Created on: May 10, 2023
 *      Author: kiraly
 */

#ifndef NETWORK_CONPOOL_H_
#define NETWORK_CONPOOL_H_

#include <pthread.h>
#include "dlinkedlist.h"
#include "concurrent_queue.h"

typedef struct
{
	/*
	 * The description of the socket. This way the ConPool can give it back to specific implementations to handle.
	 */
	int socketDescription;
	/*
	 * The queue of packets submitted by the sender, waiting for the conpool to send them.
	 */
	conc_queue* outgoingPacketQueue;

	/*
	 * When submitting a connection, one may supply an optional argument to the connection pool.
	 * This argument is used when calling the outgoing function.
	 */
	void* optionalArgument;
	/*
	 * The outgoing function to be called by the connection pool handler, when a packet is
	 * observed in the queue. This function should accept a socket descriptor, the data to be sent, and the length of the data to be sent.
	 */
	void (*outgoingFunction)(int,void*,void*);
	/*
	 * The incoming function to be called when the connection pool wants to read data from this connection. It must only accept a socket descriptor
	 * and return the data it received. This function must be a blocking function
	 */
	void* (*incomingFunction)(int);
	/*
	 * The processing function to be called after the connection pool has successfully read data from the socket. It must accept arbitrary data and return nothing.
	 */
	void (*processingFunction)(void*);

} conpool_connection_details_t;

typedef struct
{
	/*
	 * The sending thread.
	 */
	pthread_t associatedSendingThread;

	/*
	 * The receiving thread.
	 */
	pthread_t associatedReceivingThread;
	/*
	 * The connection details that are simply packed in here.
	 * DO NOT FREE FROM HERE, THIS IS PASSED FROM SOMEWHERE ELSE.
	 */
	conpool_connection_details_t* connectionDetails;
} conpool_connection_t;

typedef struct
{
	/*
	 * List of structures that remember the connection itself.
	 * NOTE: This is a structure containing references to the threads as well as various things about the connection
	 */
	dlinkedlist_t* activeConnectionThreads;
} conpool_t;

//=============GENESIS FUNCTIONS===========
/*
 * Creates a pool of connections and returns it.
 */
conpool_t* conpool_createPool();
/*
 * Aggressively destroys a pool of connections.
 * This function returns a "0" if anything went wrong destroying connecitions, or "1" otherwise.
 */
uint8_t conpool_destroyPool(conpool_t* __thePool);
//===============GENERIC FUNCTIONALITY FUNCTIONS=========

/*
 * Creates a connection for the submitter to use. It requires:
 * - The socket descriptor
 * - The outgoing function (which must accept a socket descriptor and data to send (in a custom format, provided by the implementation
 * - The incoming function (which must accept a socket descriptor, and returns data which will then be processed.
 * - The processing function (which must accept random data and return nothing).
 *
 * This function may also take an optional argument, which is supplied to the outgoing function.
 * This function may return NULL if the connection failed to be set up.
 * NOTE: Currently, connections cannot be destroyed unless the whole pool goes down.
 */
conc_queue* conpool_createConnection(conpool_t* __theConnectionPool, int __socketDescription,void (*__outgoingFunction)(int,void*),void* (*__incomingFunction)(int),void (*__processingFunction)(void*),void* __optionalArgument);

//TODO implement connection destruction (based on the pointers of the queue?)

#endif /* NETWORK_CONPOOL_H_ */
