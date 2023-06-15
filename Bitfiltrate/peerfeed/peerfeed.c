/*
 * peerfeed.c
 *
 *  Created on: Jun 9, 2023
 *      Author: kiraly
 */

#include "peerfeed.h"
#include "concurrent_queue.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PEERFEED_MAX_LINE_LEN 21

uint8_t peerfeed_ingestPeersFromFile(conc_queue_t* __peerIngestionQueue,char* __theFile)
{
	 int _theFD = open(__theFile, O_RDONLY);
//	 printf("Peerfeed is feeding from %s\n",__theFile);
//	 printf("Peerfeed ingested 1000 peers from the file.\n");
//	 printf("Swarm initialized, progressively connecting and querying.\n");
	 uint8_t _theBuffer[PEERFEED_MAX_LINE_LEN+3]; //Should fit at most 255.255.255.255:65535 , a NEWLINE and a RETURNLINE and a NULL
	 off_t _aheadJump = 0;
	 do
	 {
		 lseek(_theFD, _aheadJump * -1, SEEK_CUR); //Offset backwards, depending on where we found the newline

		 for (size_t _bufferOffset = 0; _bufferOffset < PEERFEED_MAX_LINE_LEN+3; _bufferOffset++)
		 {
			 _theBuffer[_bufferOffset] = 0;
		 }

		 size_t _readBytes = read(_theFD,_theBuffer,PEERFEED_MAX_LINE_LEN+2);
		 if (_readBytes < 0)
		 {
			 printf("DEBUG: Read file failed\n");
			 break;
		 }

		 size_t _positionEncounteredFlag = 0;
		 size_t _positionFlagEnded = 0;
		 for (size_t _bufferOffset = 0; _bufferOffset < _readBytes; _bufferOffset++)
		 {
			 if (_positionEncounteredFlag == 0 && (_theBuffer[_bufferOffset] == '\r' || _theBuffer[_bufferOffset] == '\n'))
			 {
				 _positionEncounteredFlag = _bufferOffset;
				 _positionFlagEnded = _bufferOffset;
			 }
		 }

		 _theBuffer[_positionEncounteredFlag] = 0;

		 for (size_t _bufferOffset = _positionEncounteredFlag + 1; _bufferOffset < _readBytes; _bufferOffset++) //Starting from the offending position, but after it,
		 {
			 if (_theBuffer[_bufferOffset] != '\r' && _theBuffer[_bufferOffset] != '\n')
			 {
				 _positionFlagEnded = _bufferOffset; //Remember where the first non-special charcater begins
				 break;
			 }
		 }

		 if (_readBytes == 1 || _readBytes == _positionFlagEnded)
		 {
			 printf("DEBUG: peerfeed reached end of file\n");
			 break;
		 }
		 _aheadJump = _readBytes - _positionFlagEnded;

		 //=== PROCESS THE BUFFER ===

		 uint8_t* _addressOffset = _theBuffer;
		 uint8_t* _portOffset = NULL;
		 for (size_t _bufferIterator = 0; _bufferIterator < PEERFEED_MAX_LINE_LEN+3; _bufferIterator++)
		 {
			 if (_theBuffer[_bufferIterator] == ':')
			 {
				 _theBuffer[_bufferIterator] = 0;
				 _portOffset = _addressOffset + _bufferIterator + 1;
			 }
		 }

		 //=== CREATE THE REPRESENTATION ===
		 uint16_t _thePort = atoi((char*)_portOffset);

		 struct in_addr _tmpAddress;
		 if (inet_pton(AF_INET, (char*)_addressOffset, &(_tmpAddress.s_addr)) != 1)
		 {
			 close(_theFD);
			 return 0;
		 }
		 peer_networkconfig_h* _thePeerDetails = peer_networkdetails_generatePeerDetails(_tmpAddress.s_addr,_thePort);

		 conc_queue_push(__peerIngestionQueue,_thePeerDetails);
	 }
	 while(1);
	 close(_theFD);
	 return 1;
}
