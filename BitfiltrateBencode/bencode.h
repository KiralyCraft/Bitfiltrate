/*
 * bencode.h
 *
 *  Created on: Apr 8, 2023
 *      Author: kiraly
 */

#ifndef BENCODE_H_
#define BENCODE_H_

#include <stdint.h>

typedef struct
{
	void* memoryRegion;
	uint32_t memoryLength;
	uint32_t currentOffset;
} bencode_t;


bencode_t* bencode_openFile(char* __filePath);

#endif /* BENCODE_H_ */
