/*
 * test.c
 *
 *  Created on: Apr 8, 2023
 *      Author: kiraly
 */
#include "bencode.h"
#include "bencode_internal.h"

void printSpaces(uint64_t parseDepth)
{
	for (uint64_t _depthIterator = 0; _depthIterator < parseDepth*2; _depthIterator++)
	{
		printf("_");
	}
}
void dumpElement(bencode_et* __theElement,uint64_t parseDepth)
{
	printSpaces(parseDepth);

	if (__theElement->bencodeType == BENCODE_BYTE_STRING)
	{
		bencode_et_bytestring* _theData = __theElement->bencodeSubData;
		printf("BENCODE_BYTE_STRING - %s\n",_theData->byteStringData);
	}
	else if (__theElement->bencodeType == BENCODE_INTEGER)
	{
		bencode_et_integer* _theData = __theElement->bencodeSubData;
		printf("BENCODE_INTEGER - %ld\n",_theData->integerData);
	}
	else if (__theElement->bencodeType == BENCODE_LIST)
	{
		bencode_et_list* _theData = __theElement->bencodeSubData;
		printf("BENCODE_LIST - BEGIN - DEPTH %lu\n",parseDepth);
		for (uint64_t _listIter = 0; _listIter < _theData->listElementCount; _listIter++)
		{
			dumpElement(_theData->listElementArray[_listIter],parseDepth+1);
		}
		printSpaces(parseDepth);
		printf("BENCODE_LIST - END - DEPTH %lu\n",parseDepth);
	}
	else if (__theElement->bencodeType == BENCODE_DICTIONARY)
	{
		bencode_et_dictionary* _theData = __theElement->bencodeSubData;
		printf("BENCODE_DICTINOARY - BEGIN - DEPTH %lu\n",parseDepth);
		for (uint64_t _listIter = 0; _listIter < _theData->dictionaryElementCount; _listIter++)
		{
			bencode_et_dictionary_pair* _dictionaryPair = _theData->dictionaryContents[_listIter];
			printSpaces(parseDepth);
			printf("BENCODE_DICTIONARY - BEGIN - KEY\n");
			dumpElement(_dictionaryPair->dictionaryKey,parseDepth+1);
			printSpaces(parseDepth);
			printf("BENCODE_DICTIONARY - BEGIN - VALUE\n");
			dumpElement(_dictionaryPair->dictionaryValue,parseDepth+1);
		}
		printSpaces(parseDepth);
		printf("BENCODE_DICTINOARY - END - DEPTH %lu\n",parseDepth);
	}
}
int main()
{
	bencode_t* _openTorrent = bencode_openFile("systemrescue-10.00-amd64.iso.torrent");
	printf("asdf\n");
//	bencode_t* _openTorrent = bencode_openTorrent("bencode_demo/dictionary_2.txt");
	while(bencode_canRead(_openTorrent))
	{
		bencode_et* _readElement = bencode_readNext(_openTorrent);
		dumpElement(_readElement,0);
	}
	printf("Gata\n");
}
