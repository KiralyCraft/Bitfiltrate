/*
 * watchdog_diskio.c
 *
 *  Created on: Jun 8, 2023
 *      Author: kiraly
 */

#include "watchdog_diskio.h"

#include "../../piecetracker/piecetracker.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void _watchdog_diskio_executor(void* __watchdogContext);

watchdog_diskio_t* watchdog_diskio_init(watchdog_t* __theWatchdog,piecetracker_t* __thePieceTracker)
{
	watchdog_diskio_t* _newDiskioWatchdog = malloc(sizeof(watchdog_diskio_t));
	_newDiskioWatchdog -> thePieceTracker = __thePieceTracker;
	_newDiskioWatchdog -> theFileDescriptor = open("torrent_dump", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (_newDiskioWatchdog -> theFileDescriptor == -1)
	{
		; //TODO handle file open error
	}

	//=== SUBMITTING THE WATCHDOG ===
	uint8_t _submissionResult = watchdog_submitWatchdog(_newDiskioWatchdog,_watchdog_diskio_executor,__theWatchdog);

	if (_submissionResult == 0)
	{
		//TODO handle error
	}
	return _newDiskioWatchdog;
}

void _watchdog_diskio_executor(void* __watchdogContext)
{
	watchdog_diskio_t* _watchdogData = __watchdogContext;

	piecetracker_t* _thePieceTracker = _watchdogData->thePieceTracker;
	piecetracker_pieceexport_t* _theConsumedPiece = piecetracker_consumeExportPieceIfAny(_thePieceTracker); //TODO move these into their designated diskio handler

	if (_theConsumedPiece != NULL)
	{
		int _theFD = _watchdogData->theFileDescriptor;
		piece_t* _theExportedPiece = _theConsumedPiece->theExportedPiece;
		size_t _blockLength = (1 << _theConsumedPiece->blockSizeLog2);

		for (size_t _blockIterator = 0; _blockIterator < _theExportedPiece->filledBlocks; _blockIterator++)
		{
			off_t _theDesiredOffset = _theConsumedPiece->pieceIndex * (1 << _theConsumedPiece->pieceSizeLog2) + _blockIterator * _blockLength;

			off_t offset = lseek(_theFD, _theDesiredOffset, SEEK_SET);
			if (offset == -1)
			{
				perror("Failed to seek the file");
				//TODO handle this, retry?
			}

			ssize_t bytes_written = write(_theFD, _theExportedPiece->theBlocks[_blockIterator], _blockLength);
			if (bytes_written == -1)
			{
				perror("Failed to write to file");
			}
			else if (bytes_written != _blockLength)
			{
				perror("Failed to write all bytes");
			}

			free(_theExportedPiece->theBlocks[_blockIterator]);
		}
		free(_theExportedPiece->theBlocks);
		free(_theExportedPiece->theBlockFillment);
		piecetracker_destroyExportPieceWrapper(_theConsumedPiece);
	}
//	de adaugat aici o chestiune tip "consume" din piecetracker, care ia un piece care e full si il pune used, apoi scrie pe disk
//	datele din el (acolo unde treubie, cu un fseek) si apoi da free la datele din pachet, ca o venit de la comms
}
