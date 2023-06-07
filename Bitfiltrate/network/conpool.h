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

typedef enum
{
	CONPOOL_CONNECTION_STATUS_ALIVE,
	CONPOOL_CONNECTION_STATUS_DEAD,
} conpool_connection_status_e;
typedef struct
{
	/*
	 * The description of the socket. This way the ConPool can give it back to specific implementations to handle.
	 */
	void* socketDescription;
	/*
	 * The queue of packets submitted by the sender, waiting for the ConPool to send them.
	 */
	conc_queue* outgoingPacketQueue;

	/*
	 * The execution context which is passed to the processing function as a bundle.
	 */
	void* executionContext;
	/*
	 * When submitting a connection, one may supply an optional argument to the connection pool.
	 * This argument is used when calling the outgoing function.
	 */
	void* optionalArgument;
	/*
	 * The outgoing function to be called by the connection pool handler, when a packet is
	 * observed in the queue. This function should accept a socket descriptor, the data to be sent, and the length of the data to be sent.
	 */
	void (*outgoingFunction)(void*,void*,void*);
	/*
	 * The incoming function to be called when the connection pool wants to read data from this connection. It must only accept a socket descriptor
	 * and return the data it received. This function must be a blocking function
	 */
	void* (*incomingFunction)(void*,void*);
	/*
	 * The processing function to be called after the connection pool has successfully read data from the socket. It must accept arbitrary data and may return data.
	 *
	 * The returned data, however, is not used for now, but rather the signature is required for the processing thread to accept.
	 *
	 * //TODO make this function accept three parameters, instead of having them bundled in a void
	 */
	void* (*processingFunction)(void*);

	/*
	 * This variable represents the status of the connection, be it dead or alive, or another yet undocumented status.
	 */
	conpool_connection_status_e connectionHealth;

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
 * 	 The outgoing function must also clear the memory allocated to it's packets as soon as they are sent.
 *
 * - The incoming function (which must accept a socket descriptor, and returns data which will then be processed.
 * - The processing function (which must accept random data and return nothing). Note that each processing of a packet is done in a different thread, which
 *   only lives for the duration of the processing event.
 *
 *   The processing function is also given a bundle of data, of which the first argument (of type void*) is the data to process, and the third one is
 *   the optional arguments which is passed to the processing function. The actual implementation of processing must free this data bundle after using it.
 *   The processing function must also clear the memory allocated to it's packets as soon as they are processed.
 * - The execution context that is only passed to the processing function as the second argument of the bundle.
 *
 * This function may also take an optional argument, which is supplied all functions.
 * This function may return NULL if the connection failed to be set up.
 * NOTE: Currently, connections cannot be destroyed unless the whole pool goes down.
 */
conc_queue* conpool_createConnection(conpool_t* __theConnectionPool, void* __socketDescription,void (*__outgoingFunction)(void*,void*,void*),void* (*__incomingFunction)(void*,void*),void* (*__processingFunction)(void*),void* __executionContext,void* __optionalArgument);

//TODO implement connection destruction (based on the pointers of the queue?)

#endif /* NETWORK_CONPOOL_H_ */
