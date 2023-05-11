/*
 * conpool_executor.h
 *
 *  Created on: May 11, 2023
 *      Author: kiraly
 */

#ifndef NETWORK_CONPOOL_EXECUTOR_H_
#define NETWORK_CONPOOL_EXECUTOR_H_

/*
 * The thread function that takes care of sending data from the queue.
 */
void* conpool_executorSendingFunction(void* __providedData);
/*
 * The thread function that takes care of receiving data from the socket and having it processed.
 */
void* conpool_executorReceivingFunction(void* __providedData);



#endif /* NETWORK_CONPOOL_EXECUTOR_H_ */
