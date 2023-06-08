/*
 * swarm_query.h
 *
 *  Created on: Jun 7, 2023
 *      Author: kiraly
 */

#ifndef SWARM_SWARM_QUERY_H_
#define SWARM_SWARM_QUERY_H_

typedef enum
{
	/*
	 * Used to indicate a peer is queried for it's current incoming block count.
	 *
	 * This query should return a size_t
	 */
	SWARM_QUERY_BLOCK_COUNT,
	/*
	 * Used to query a peer for whether or not they have a certain piece.
	 *
	 * This query should return a boolean (uint8_t)
	 */
	SWARM_QUERY_HAS_PIECE,
	/*
	 * Used to fetch (drain) one piece from the queue of this peer, regardless which.
	 *
	 * This query should return a void* representing the piece data, specific to the implementation.
	 */
	SWARM_QUERY_DRAIN_PIECE,

	/*
	 * Used to query a peer for the highest available piece that it has, indicating
	 * a possible length of the torrent.
	 *
	 * This query should return a size_t
	 */
	SWARM_QUERY_HIGHEST_BITFIELD_PIECE
} swarm_query_type_e;

#endif /* SWARM_SWARM_QUERY_H_ */
