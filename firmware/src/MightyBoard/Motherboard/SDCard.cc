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

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "SDCard.hh"
#include "Motherboard.hh"
#include <avr/io.h>
#include <string.h>
#include "lib_sd/sd-reader_config.h"
#include "lib_sd/fat.h"
#include "lib_sd/sd_raw.h"
#include "lib_sd/partition.h"
#include "Menu_locales.hh"

#ifndef USE_DYNAMIC_MEMORY
#error Dynamic memory should be explicitly disabled in the G3 mobo.
#endif

#if (USE_DYNAMIC_MEMORY == 1)
#error Dynamic memory should be explicitly disabled in the G3 mobo.
#endif

namespace sdcard {

struct partition_struct* partition = 0;
struct fat_fs_struct* fs = 0;
struct fat_dir_struct* dd = 0;
struct fat_file_struct* file = 0;

bool openPartition()
{
  /* open first partition */
  partition = partition_open(sd_raw_read,
                             sd_raw_read_interval,
                             sd_raw_write,
                             sd_raw_write_interval,
                             0);
                             
  if(!partition)
  {
    /* If the partition did not open, assume the storage device
    * is a "superfloppy", i.e. has no MBR.
    */
    partition = partition_open(sd_raw_read,
                               sd_raw_read_interval,
                               sd_raw_write,
                               sd_raw_write_interval,
                               -1);
  }
  if(!partition)
    return false;
  return true;
}

bool openFilesys()
{
  /* open file system */
  fs = fat_open(partition);
  return fs != 0;
}

bool openRoot()
{
  // Open root directory
  struct fat_dir_entry_struct rootdirectory;
  fat_get_dir_entry_of_path(fs, "/", &rootdirectory);
  dd = fat_open_dir(fs, &rootdirectory);
  return dd != 0;
}

bool checkVolumeSize(){
#if SD_RAW_SDHC 
	return true;
#else
	return fat_get_fs_size(fs) < 2147483648; //2GB
#endif
}


SdErrorCode initCard() {
	if (!sd_raw_init()) {;
		if (!sd_raw_available()) {
			reset();
			return SD_ERR_NO_CARD_PRESENT;
		} else {
			reset();
			return SD_ERR_INIT_FAILED;
		}
	} else if (!openPartition()) {
		reset();
		return SD_ERR_PARTITION_READ;
	} else if (!openFilesys()) {
		reset();
		return SD_ERR_OPEN_FILESYSTEM;
	} else if (!openRoot()) {
		reset();
		return SD_ERR_NO_ROOT;
	} else if (!checkVolumeSize()){
		reset();
		return SD_ERR_VOLUME_TOO_BIG;
		
	/* we need to keep locked as the last check */
	} else if (sd_raw_locked()) {
		return SD_ERR_CARD_LOCKED;
	}
	return SD_SUCCESS;
}

SdErrorCode directoryReset() {
  reset();
  SdErrorCode rsp = initCard();
  if (rsp != SD_SUCCESS && rsp != SD_ERR_CARD_LOCKED) {
    return rsp;
  }
  fat_reset_dir(dd);
  return SD_SUCCESS;
}

SdErrorCode directoryNextEntry(char* buffer, uint8_t bufsize, uint8_t * fileLength) {
	struct fat_dir_entry_struct entry;
	// This is a bit of a hack.  For whatever reason, some filesystems return
	// files with nulls as the first character of their name.  This isn't
	// necessarily broken in of itself, but a null name is also our way
	// of signalling we've gone through the directory, so we discard these
	// entries.  We have an upper limit on the number of entries to cycle
	// through, so we don't potentially lock up here.
	uint8_t tries = 5;
	while (tries) {
		if (fat_read_dir(dd, &entry)) {
			int i;
			for (i = 0; (i < bufsize-1) && entry.long_name[i] != 0; i++) {
				buffer[i] = entry.long_name[i];
			}
			buffer[i] = 0;
			if (i > 0) {
				if(fileLength != 0)
					*fileLength = i;
				break;
			} else {
				tries--;
			}
		} else {
			buffer[0] = 0;
			break;
		}
	}
	return SD_SUCCESS;
}

bool findFileInDir(const char* name, struct fat_dir_entry_struct* dir_entry)
{
  while(fat_read_dir(dd, dir_entry))
  {
    if(strcmp(dir_entry->long_name, name) == 0)
    {
      fat_reset_dir(dd);
      return true;
    }
  }
  return false;
}

bool openFile(const char* name, struct fat_file_struct** file)
{
  struct fat_dir_entry_struct fileEntry;
  if(!findFileInDir(name, &fileEntry))
  {
    return false;
  }

  *file = fat_open_file(fs, &fileEntry);
  return true;
}

uint32_t getFileSize(){
	return fat_get_file_size(file);
}

bool deleteFile(char *name)
{
  struct fat_dir_entry_struct fileEntry;
  if(!findFileInDir(name, &fileEntry))
  {
    return false;
  }
  fat_delete_file(fs, &fileEntry);
  return true;
}

bool createFile(char *name)
{
  struct fat_dir_entry_struct fileEntry;
  return fat_create_file(dd, name, &fileEntry) != 0;
}

bool capturing = false;
bool playing = false;
uint32_t capturedBytes = 0L;

bool isPlaying() {
	return playing;
}

bool isCapturing() {
	return capturing;
}

SdErrorCode startCapture(char* filename)
{
  reset();
  SdErrorCode result = initCard();
  if (result != SD_SUCCESS) {
    return result;
  }
  capturedBytes = 0L;
  file = 0;
  // Always operate in truncation mode.
  deleteFile(filename);
  if (!createFile(filename)) {
    return SD_ERR_FILE_NOT_FOUND;
  }

  if (!openFile(filename,&file)) {
    return SD_ERR_PARTITION_READ;
  }
  if (file == 0) {
    return SD_ERR_GENERIC;
  }

  capturing = true;
  return SD_SUCCESS;
}

void capturePacket(const Packet& packet)
{
	if (file == 0) return;
	// Casting away volatile is OK in this instance; we know where the
	// data is located and that fat_write_file isn't caching
	fat_write_file(file, (uint8_t*)packet.getData(), packet.getLength());
	capturedBytes += packet.getLength();
}


uint32_t finishCapture()
{
  if (capturing) {
    if (file != 0) {
    	fat_close_file(file);
    	sd_raw_sync();
    }
    file = 0;
    capturing = false;
  }
  //reset();
  return capturedBytes;
}

uint8_t next_byte;
bool has_more;
bool retry;

void fetchNextByte() {
  if(sd_raw_available()){
	int16_t read = fat_read_file(file, &next_byte, 1);
	//retry = read < 0;
	has_more = read > 0;
  }else{
	Motherboard::getBoard().errorResponse(ERROR_SD_CARD_REMOVED, true);
	has_more = 0;
	retry = 0;
	}
}

bool playbackRetry() {
	return retry;
}

bool playbackHasNext() {
  return has_more;// || retry;
}

uint8_t playbackNext() {
  uint8_t rv = next_byte;
  fetchNextByte();
  return rv;
}

SdErrorCode startPlayback(char* filename) {
  reset();
  SdErrorCode result = initCard();
  /* for playback it's ok if the card is locked */
  if (result != SD_SUCCESS && result != SD_ERR_CARD_LOCKED) {
    return result;
  }
  capturedBytes = 0L;
  file = 0;
  if (!openFile(filename, &file) || file == 0) {
    return SD_ERR_FILE_NOT_FOUND;
  }
  playing = true;
  fetchNextByte();
  return SD_SUCCESS;
}

void playbackRewind(uint8_t bytes) {
  int32_t offset = -((int32_t)bytes);
  fat_seek_file(file, &offset, FAT_SEEK_CUR);
}

void finishPlayback() {
  playing = false;
  if (file != 0) {
	  fat_close_file(file);
	  sd_raw_sync();
  }
  file = 0;
}


void reset() {
	if (playing)
		finishPlayback();
	if (capturing)
		finishCapture();
	if (dd != 0) {
		fat_close_dir(dd);
		dd = 0;
	}
	if (fs != 0) {
		fat_close(fs);
		fs = 0;
	}
	if (partition != 0) {
		partition_close(partition);
		partition = 0;
	}
}

} // namespace sdcard
