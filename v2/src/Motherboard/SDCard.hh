/*
 * Copyright 2010 by Adam Mayer <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */


#ifndef SDCARD_HH_
#define SDCARD_HH_

#include <stdint.h>
#include "Packet.hh"

namespace sdcard {

/**
 * This enumeration lists all the SD card call error/success codes.
 * Any non-zero value is an error condition.
 */
typedef enum {
  SD_SUCCESS              = 0,  // Operation succeeded
  SD_ERR_NO_CARD_PRESENT  = 1,  // No SD card is inserted in the slot
  SD_ERR_INIT_FAILED      = 2,  // SD card initialization failed
  SD_ERR_PARTITION_READ   = 3,  // Couldn't read the card's partition table
  SD_ERR_OPEN_FILESYSTEM  = 4,  // Couldn't open the FAT16 filesystem --
                                //  check that it's real FAT16
  SD_ERR_NO_ROOT          = 5,  // No root directory found
  SD_ERR_CARD_LOCKED      = 6,  // Card is locked, writing forbidden
  SD_ERR_FILE_NOT_FOUND   = 7,  // Could not find specific file
  SD_ERR_GENERIC          = 8   // General error
} SdErrorCode;

/**
 * Reset the SD card subsystem.
 */
void reset();

/**
 * Start a directory scan.
 */
SdErrorCode directoryReset();
/**
 * Get the next filename in a directory scan.
 */
SdErrorCode directoryNextEntry(char* buffer, uint8_t bufsize);

/**************************/
/** Build Capture         */
/**************************/

// Begin capturing bufffered commands to a new file with the given filename.
// Returns an SD card error/success code.
SdErrorCode startCapture(char* filename);
// Capture the contents of a packet to the currently open file.
void capturePacket(const Packet& packet);
// Complete the capture, and flush buffers.  Return the number of bytes
// written to the card.
uint32_t finishCapture();
// True if we're capturing buffered commands to a file, false otherwise
bool isCapturing();



/**************************/
/** Build Playback        */
/**************************/

// Begin playing back commands from a file on the SD card.
// Returns an SD card error/success code.
SdErrorCode startPlayback(char* filename);
// See if there is more data available in the playback file.
bool playbackHasNext();
// Return the next byte from the currently open file.
uint8_t playbackNext();
// Rewind the given number of bytes in the input stream.
void playbackRewind(uint8_t bytes);
// Halt playback.  Should be called at the end of playback, or on manual
// halt; frees up resources.
void finishPlayback();
// True if we're playing back buffered commands from a file, false otherwise
bool isPlaying();

} // namespace sdcard

#endif // SDCARD_HH_
