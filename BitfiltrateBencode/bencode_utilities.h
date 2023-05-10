/*
 * bencode_utilities.h
 *
 *  Created on: May 9, 2023
 *      Author: kiraly
 */

#ifndef BENCODE_UTILITIES_H_
#define BENCODE_UTILITIES_H_
#include "bencode_internal.h"

/*
 * When provided with a bencode dictionary and a string-based key, it attempts to retrieve the value of said key from
 * the dictionary. If it does not exist, it returns NULL.
 */
bencode_et* bencode_readStringDictKey(char* __dictionaryKey, bencode_et* __theDictionary);

#endif /* BENCODE_UTILITIES_H_ */
