/*
 * udptracker_comm.c
 *
 *  Created on: May 20, 2023
 *      Author: kiraly
 */

#include "udptracker_comm.h"



void udptracker_comm_outgoingFunction(int __socketDescriptor, void* __outgoingData,void* __optionalArgument)
{
	printf("Got something to send :)");
}

void* udptracker_comm_incomingFunction(int __socketDescriptor)
{

}

void udptracker_comm_processingFunction(void* __dataToProcess)
{

}
