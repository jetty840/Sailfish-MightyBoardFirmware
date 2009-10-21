#include <avr/io.h>
#include "RepRapSDCard.h"
#include "fat.h"
#include "sd_raw.h"
#include "partition.h"
#include <string.h>

#ifndef USE_DYNAMIC_MEMORY
#error Dynamic memory should be explicitly disabled for the RepRapSDCard implementation.
#endif

#if (USE_DYNAMIC_MEMORY == 1)
#error Dynamic memory should be disabled for the RepRapSDCard implementation.
#endif

#if (USE_DYNAMIC_MEMORY == 0)
  struct partition_struct partition_handles[PARTITION_COUNT];
#endif

RepRapSDCard::RepRapSDCard() :
  partition(NULL),
  fs(NULL),
  dd(NULL)
{
}

void RepRapSDCard::reset() {
  if (dd != NULL) {
    fat_close_dir(dd);
    dd = NULL;
  }
  if (fs != NULL) {
    fat_close(fs);
    fs = NULL;
  }
  if (partition != NULL) {
    partition_close(partition);
    partition = NULL;
  }
}

uint8_t RepRapSDCard::init_card(void)
{
  return sd_raw_init();
}

bool RepRapSDCard::isLocked(void)
{
  return sd_raw_locked() == 1;
}

bool RepRapSDCard::isAvailable(void)
{
  return sd_raw_available();
}


uint8_t RepRapSDCard::open_partition(void)
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
    return 0;
  return 1;
}

uint8_t RepRapSDCard::open_filesys(void)
{
  /* open file system */
  fs = fat_open(partition);
  if(!fs)
    return 0;

  return 1;
}

uint8_t RepRapSDCard::open_root()
{
  // Open root directory
  struct fat_dir_entry_struct rootdirectory;

  fat_get_dir_entry_of_path(fs, "/", &rootdirectory);
  dd = fat_open_dir(fs, &rootdirectory);
  if(!dd)
    return 0;

  return 1;
}


uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)
{
  while(fat_read_dir(dd, dir_entry))
  {
    if(strcmp(dir_entry->long_name, name) == 0)
    {
      fat_reset_dir(dd);
      return 1;
    }
  }

  return 0;
}

uint8_t open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name,
					 File *file)
{
  struct fat_dir_entry_struct file_entry;

  if(!find_file_in_dir(fs, dd, name, &file_entry))
  {
    //Serial.println("File not found");
    return 0;
  }

  *file = fat_open_file(fs, &file_entry);
  return 1;
}


uint8_t RepRapSDCard::open_file(char *name, File *file)
{
  return open_file_in_dir(fs, dd, name, file);
}


uint8_t RepRapSDCard::delete_file(char *name)
{
  struct fat_dir_entry_struct file_entry;
  if(!find_file_in_dir(fs, dd, name, &file_entry))
  {
    return 0;
  }
  fat_delete_file(fs, &file_entry);
  return 1;
}

uint8_t RepRapSDCard::create_file(char *name)
{
  struct fat_dir_entry_struct file_entry;
  return fat_create_file(dd, name, &file_entry);
}

uint8_t RepRapSDCard::reset_file(File f)
{
  return fat_seek_file(f, 0, FAT_SEEK_SET);
}

uint8_t RepRapSDCard::seek_file(File f, int32_t *offset, uint8_t whence)
{
  return fat_seek_file(f, offset, whence);
}

uint16_t RepRapSDCard::read_file(File f, uint8_t* buffer, uint16_t buffer_len)
{
  return fat_read_file(f, buffer, buffer_len);
}

uint8_t RepRapSDCard::write_file(File f, uint8_t *buff, uint8_t siz) 
{
  return fat_write_file(f, buff, siz);
}

void RepRapSDCard::close_file(File f)
{
  //fat_resize_file(f,f->pos);
  fat_close_file(f);
  sd_raw_sync();
}

uint8_t RepRapSDCard::sd_scan_reset() {
  fat_reset_dir(dd);
  return 0;
}

uint8_t RepRapSDCard::sd_scan_next(char* buffer, uint8_t bufsize) {
  struct fat_dir_entry_struct entry;
  if (fat_read_dir(dd, &entry)) {
    int i;
    for (i = 0; (i < bufsize-1) && entry.long_name[i] != 0; i++) {
      buffer[i] = entry.long_name[i];
    }
    buffer[i] = 0;
  } else {
    buffer[0] = 0;
  }
  return 0;
}

