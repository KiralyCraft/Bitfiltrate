/*
 * torrentinfo.h
 *
 *  Created on: May 9, 2023
 *      Author: kiraly
 */

#ifndef TORRENTINFO_TORRENTINFO_H_
#define TORRENTINFO_TORRENTINFO_H_

#include "bencode.h"
#include "bencode_internal.h"
#include "stdint.h"

typedef struct
{
	/*
	 * Keep track of the file itself, as represented in memory.
	 */
	bencode_t* fileRepresentation;

	/*
	 * DEBUG: Keep the root handle of this torrent file.
	 */
	bencode_et* rootHandle;
	/*
	 * info
	 */
	bencode_et_dictionary* infoDictionary;
	/*
	 * announce
	 */
	bencode_et_bytestring* announceString;
	/*
	 * announce-list (optional)
	 */
	bencode_et_list* announceList;

	//================EXTRA DATA THAT MAYBE SHOULD BE IN THEIR OWN STRUCT=====================

	uint8_t torrentHash[20];

} torrent_t;

torrent_t* openTorrent(char* __filePath);

#endif /* TORRENTINFO_TORRENTINFO_H_ */
