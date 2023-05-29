/*
 * tcpswarm.c
 *
 *  Created on: May 27, 2023
 *      Author: kiralycraft
 */

#include "tcpswarm.h"
#include <stdlib.h>

swarm_definition_t* tcpswarm_createDefinition()
{
	swarm_definition_t* _theSwarmDefinition = malloc(sizeof(swarm_definition_t));
	return _theSwarmDefinition;
}

