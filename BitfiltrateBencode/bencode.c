/*
 * bencode.c
 *
 *  Created on: Apr 8, 2023
 *      Author: kiraly
 */

#include "bencode.h"
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include "bencode_errors.h"
#include <stdlib.h>
#include "bencode_internal.h"

bencode_t* bencode_openFile(char* __filePath)
{
	int _fileDescriptor = open(__filePath,O_RDONLY);
	if (_fileDescriptor < 0)
	{
		printf("ERR - Could not open the given torrent file. Error code: %s\n",strerror(errno));
		return BENCODE_FAILED_OPEN;
	}
	else
	{
		struct stat _fileStats;
		fstat(_fileDescriptor, &_fileStats);
		off_t _fileSize = _fileStats.st_size;

		void* mappingResult = mmap(NULL,_fileSize,PROT_READ,MAP_SHARED,_fileDescriptor,0);

		if (mappingResult == MAP_FAILED)
		{
			printf("ERR - Could not map the file into memory: %s\n",strerror(errno));
			return BENCODE_FAILED_MAP;
		}
		else
		{
			bencode_t* _toReturn = malloc(sizeof(bencode_t));
			_toReturn -> memoryLength = _fileSize;
			_toReturn -> memoryRegion = mappingResult;
			_toReturn -> currentOffset = 0;

			return _toReturn;
		}
	}
}


