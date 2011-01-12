#ifndef _SD_SUPPORT_H_
#define _SD_SUPPORT_H_

#define USE_SD_CARD

/**
 * This enumeration lists all the SD card call error/success codes.
 * Any non-zero value is an error condition.
 */
enum {
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
};


#include <stdint.h>
#include <SimplePacket.h>

// True if motherboard is currently capturing a build to a file.
extern bool capturing;
// True if the motherboard is currently playing back a build from a 
// file on the SD card.
extern bool playing;

// True if we're capturing buffered commands to a file, false otherwise
inline bool is_capturing() { return capturing; }
// True if we're playing back buffered commands from a file, false otherwise
inline bool is_playing() { return playing; }

/**************************/
/** Build Capture         */
/**************************/

// Begin capturing bufffered commands to a new file with the given filename.
// Returns an SD card error/success code.
uint8_t start_capture(char* filename);
// Capture the contents of a packet to the currently open file.
void capture_packet(SimplePacket& packet);
// Complete the capture, and flush buffers.  Return the number of bytes
// written to the card.
uint32_t finish_capture();

/**************************/
/** Build Playback        */
/**************************/

// Begin playing back commands from a file on the SD card.
// Returns an SD card error/success code.
uint8_t start_playback(char* filename);
// See if there is more data available in the playback file.
bool playback_has_next();
// Return the next byte from the currently open file.
uint8_t playback_next();
// Rewind the given number of bytes in the input stream.
void playback_rewind(uint8_t bytes);
// Halt playback.  Should be called at the end of playback, or on manual
// halt; frees up resources.
void finish_playback();

void sd_reset();

uint8_t sd_scan_reset();
uint8_t sd_scan_next(char* buffer, uint8_t bufsize);

#endif // _SD_SUPPORT_H_
