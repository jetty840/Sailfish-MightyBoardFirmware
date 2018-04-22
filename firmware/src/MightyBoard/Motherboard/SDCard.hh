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

/// Interface to the SD card library. Provides straightforward functions for
/// listing directory contents, and reading and writing jobs to files.
namespace sdcard {

#ifdef PLATFORM_SD_READ_BUFFER
#define SD_BYTE_BUFLEN PLATFORM_SD_READ_BUFFER
#else
#define SD_BYTE_BUFLEN 32
#endif

    /// This enumeration lists all the SD card call error/success codes.
    /// Any non-zero value is an error condition.
    typedef enum {
      SD_SUCCESS              = 0,  ///< Operation succeeded
      SD_ERR_NO_CARD_PRESENT  = 1,  ///< No SD card is inserted in the slot
      SD_ERR_INIT_FAILED      = 2,  ///< SD card initialization failed
      SD_ERR_PARTITION_READ   = 3,  ///< Couldn't read the card's partition table
      SD_ERR_OPEN_FILESYSTEM  = 4,  ///< Couldn't open the FAT16 filesystem --
                                    ///<  check that it's real FAT16
      SD_ERR_NO_ROOT          = 5,  ///< No root directory found
      SD_ERR_CARD_LOCKED      = 6,  ///< Card is locked, writing forbidden
      SD_ERR_FILE_NOT_FOUND   = 7,  ///< Could not find specific file
      SD_ERR_GENERIC          = 8,  ///< General error
      SD_CWD                  = 9,  ///< Call to playback changed the working dir
      SD_ERR_VOLUME_TOO_BIG   = 10, ///< the SD card filesystem is too large
      SD_ERR_CRC              = 11, ///< CRC check failed
      SD_ERR_READ             = 12, ///< SD card read error
      SD_ERR_DEGRADED         = 13, ///< SD card comms only working at low speeds
#if defined(DEBUG_SD)
      SD_ERR_1                = 14,
      SD_ERR_2                = 15,
      SD_ERR_3                = 16,
      SD_ERR_4                = 17,
      SD_ERR_5                = 18,
      SD_ERR_6                = 19,
      SD_ERR_7                = 20
#endif

    } SdErrorCode;

    extern SdErrorCode sdAvailable;
    extern uint8_t     sdErrno;
#ifndef BROKEN_SD
    extern volatile bool mustReinit;
#endif

    /// Reset the SD card subsystem.
    void reset();


    /// Start a directory scan.
    /// \return SD_SUCCESS if successful
    SdErrorCode directoryReset();


    /// Get the next filename in a directory scan.
    /// Returns an empty string when there are no more files
    /// \param[in] buffer Character buffer to store name in
    /// \param[in] bufsize Size of buffer
    void directoryNextEntry(char* buffer, uint8_t bufsize,
			    uint8_t* fileLength = 0, bool *isDir = 0);


    /// Begin capturing bufffered commands to a new file with the given filename.
    /// Returns an SD card error/success code.
    /// \param[in] filename Name of file to write to
    /// \return SD_SUCCESS if successful
    SdErrorCode startCapture(char* filename);


    /// Capture the contents of a packet to the currently open file.
    /// \param[in] packet Packet to write to file.
    void capturePacket(const Packet& packet);

#ifdef EEPROM_MENU_ENABLE
    /// Writes b to the open file
    bool writeByte(uint8_t b);
#endif

    /// Complete the capture, and flush buffers.  Return the number of bytes
    /// written to the card.
    /// \return Number of bytes written to the card.
    uint32_t finishCapture();


    /// Check whether a job is being captured to SD card
    /// \return True if we're capturing buffered commands to a file, false otherwise
    bool isCapturing();


    /// Begin playing back commands from a file on the SD card.
    /// Returns an SD card error/success code
    /// \param[in] filename Name of file to write to
    /// \return SD_SUCCESS if successful
    SdErrorCode startPlayback(char* filename);


    /// See if there is more data available in the playback file.
    /// \return True if there is more data in the file
    bool playbackHasNext();


    /// Return the next byte from the currently open file.
    /// \return The next byre in the file.
    uint8_t playbackNext();


    /// Halt playback.  Should be called at the end of playback, or on manual
    /// halt; frees up resources.
    void finishPlayback();

    /// Check whether a job is being played back from the SD card
    /// \return True if we're playing back buffered commands from a file, false otherwise
    bool isPlaying();

    /// Return true if file name exists on the SDCard
    bool fileExists(const char* name);

    /// Force the SD and FAT16 file system to be re-initialized
    /// and set the root directory as the current working directory
    void forceReinit();

    /// Change our current working directory to the specified directory
    bool changeDirectory(const char *name);

} // namespace sdcard

#endif // SDCARD_HH_
