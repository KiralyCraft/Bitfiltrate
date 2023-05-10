/*
 * torrentinfo.c
 *
 *  Created on: May 9, 2023
 *      Author: kiraly
 */

#include "torrentinfo.h"
#include "bencode_utilities.h"
#include <openssl/evp.h>

/**
 * Given a torrent file and the "info" dictionary within (of which only the start and end offsets are important, actually)
 */
void _torrentCreateHash(torrent_t* __torrentData, bencode_et* __infoDictionaryValue)
{
	uint64_t _dataStartOffset = __infoDictionaryValue->dataStartOffset;
	uint64_t _dataEndOffset = __infoDictionaryValue->dataEndOffset;
	uint64_t _dataLength = _dataEndOffset - _dataStartOffset + 1;

	EVP_MD_CTX* _hashContext = EVP_MD_CTX_new();
    EVP_DigestInit_ex(_hashContext, EVP_sha1(), NULL);
    EVP_DigestUpdate (_hashContext, __torrentData->fileRepresentation->memoryRegion + _dataStartOffset, _dataLength);

    uint32_t _hashLength;
    EVP_DigestFinal_ex (_hashContext, __torrentData->torrentHash, &_hashLength);
    EVP_MD_CTX_free(_hashContext);
}

torrent_t* openTorrent(char* __filePath)
{
	torrent_t* _torrentData = malloc(sizeof(torrent_t));

	_torrentData->fileRepresentation = bencode_openFile(__filePath);
	_torrentData->rootHandle = bencode_readNext(_torrentData->fileRepresentation);

	if (_torrentData->rootHandle->bencodeType != BENCODE_DICTIONARY)
	{
		//TODO Handle the scenario where the provided file is not a proper torrent
	}
	else
	{
		bencode_et* _infoDictionaryValue = bencode_readStringDictKey("info",_torrentData->rootHandle);
		if (_infoDictionaryValue == NULL)
		{
			//TODO handle situation where the torrent doesn't have an info field
		}
		else
		{
			_torrentData->infoDictionary = _infoDictionaryValue->bencodeSubData;
		}

		bencode_et* _announceDictionaryValue = bencode_readStringDictKey("announce",_torrentData->rootHandle);
		if (_infoDictionaryValue == NULL)
		{
			//TODO handle situation where the torrent doesn't have an info field
		}
		else
		{
			_torrentData->announceString = _announceDictionaryValue->bencodeSubData;
		}

		bencode_et* _announceDictionaryList = bencode_readStringDictKey("announce-list",_torrentData->rootHandle);
		if (_announceDictionaryList == NULL)
		{
			//TODO handle situation where the torrent doesn't have an info field
		}
		else
		{
			_torrentData->announceList = _announceDictionaryList->bencodeSubData;
		}

		//===================PROCESSING SOME INFO FOR THE TORRENT=============================

		_torrentCreateHash(_torrentData,_infoDictionaryValue);

		return _torrentData;
	}

	return NULL;
}
