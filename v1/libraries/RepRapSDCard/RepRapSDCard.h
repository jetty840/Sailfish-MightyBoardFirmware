#ifndef _REPRAPSDCARD_H_
#define _REPRAPSDCARD_H_

#include "sd-reader_config.h"
#include "sd_raw.h"
#include "sd_raw_config.h"
#include "partition.h"
#include "partition_config.h"
#include "fat.h"

typedef struct fat_file_struct * File;

class RepRapSDCard
{
  //private?
  struct partition_struct *partition;
  struct fat_fs_struct* fs;
  struct fat_dir_struct* dd;

 public:
  RepRapSDCard();
  void reset();

  bool isAvailable(void);
  bool isLocked(void);
  uint8_t init_card(void);
  uint8_t open_partition(void);
  uint8_t open_filesys(void);
  uint8_t open_root();
  uint8_t open_file(char *name,File *file);
  void close_file(File f);
  uint8_t delete_file(char *name);
  uint8_t create_file(char *name);
  uint8_t seek_file(File fd, int32_t *offset, uint8_t whence);
  uint8_t reset_file(File f);
  uint16_t read_file(File f, uint8_t* buffer, uint16_t buffer_len);
  uint8_t write_file(File f, uint8_t *b, uint8_t num);

  uint8_t sd_scan_reset();
  uint8_t sd_scan_next(char* buffer, uint8_t bufsize);
};

#endif
