/*
 * bencode_internal.h
 *
 *  Created on: Apr 8, 2023
 *      Author: kiraly
 */

#ifndef BENCODE_INTERNAL_H_
#define BENCODE_INTERNAL_H_

#include "bencode.h"

typedef enum
{
	BENCODE_BYTE_STRING,
	BENCODE_INTEGER,
	BENCODE_LIST,
	BENCODE_DICTIONARY,
	BENCODE_TERMINATOR,
	BENCODE_SEPARATOR
} bencode_e;

typedef struct
{
	void* byteStringData;
	uint64_t byteStringDataLength;
} bencode_et_bytestring;
typedef struct
{
	int64_t integerData;
} bencode_et_integer;

typedef struct
{
	bencode_e bencodeType;
	void* bencodeSubData;
} bencode_et;

/**
 * Probes the next entity to see what it would be.
 * It makes the assumption that the offset is always right on a type boundary.
 */
bencode_e bencode_probeType(bencode_t* __providedBencode);
/**
 * Returns a generic structure which is representative of the next entity.
 * TODO: This method is currently recursive in some scenarios. Add a "pending resume offset" field and add the ability to be resumed later on.
 */
bencode_et* bencode_readNext(bencode_t* __providedBencode);

#endif /* BENCODE_INTERNAL_H_ */
