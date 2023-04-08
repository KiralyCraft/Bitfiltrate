/*
 * test.c
 *
 *  Created on: Apr 8, 2023
 *      Author: kiraly
 */
#include "bencode.h"
#include "bencode_internal.h"

int main()
{
//	bencode_t* _openTorrent = bencode_openTorrent("systemrescue-10.00-amd64.iso.torrent");
	bencode_t* _openTorrent = bencode_openTorrent("bencode_demo/bytestring.txt");
	bencode_et* _returned = bencode_readNext(_openTorrent);
	bencode_et_bytestring* asdf = _returned->bencodeSubData;
	printf("%s\n",asdf->byteStringData);
//
	bencode_et* _returnedI = bencode_readNext(_openTorrent);
	bencode_et_integer* asdff = _returnedI->bencodeSubData;
	printf("%d\n",asdff->integerData);
}
