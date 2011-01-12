#include "Configuration.h"
#include <RepRapSDCard.h>

RepRapSDCard sdcard;
File file;


bool create_slam_file(char* filename) {
  sdcard.delete_file(filename);
  uint8_t rc = sdcard.create_file(filename);
  if (rc == 0) {
    Serial.println("Could not create file");
    return false;
  }
  return true;
}

bool open_slam_file(char* filename) 
{
  uint8_t rc = sdcard.open_file(filename,&file);
  if (rc == 0) {
    Serial.println("Could not open file");
    return false;
  }
  if (file == NULL) {
    Serial.println("File is null");
    return false;
  }
  return true;
}

void close_slam_file()
{
  if (file != NULL) {
    sdcard.close_file(file);
  }
}

/*
void capture_byte(uint8_t b) {
  if (file != NULL) {
    sdcard.write_file(file, &b, 1);
    ++capturedBytes;
  }
}

void fetch_next_byte() {
  int16_t read = fat_read_file(file, &next_byte, 1);
  has_more = read > 0;
}
*/

bool open_sd() {
  sdcard.init_card();
  sdcard.reset();
  if (!sdcard.init_card()) {
    if (!sdcard.isAvailable()) {
      Serial.println("No SD card available");
    }
    else
    {
      Serial.println("SD card init failed");
    }
    return false;
  }
  else if (!sdcard.open_partition())
  {
    Serial.println("SD partition open failed");
    return false;
  }
  else if (!sdcard.open_filesys())
  {
    Serial.println("SD filesystem open failed");
    return false;
  }
  else if (!sdcard.open_root())
  {
    Serial.println("SD root directory open failed");
    return false;
  }
  else if (sdcard.isLocked())
  {
    Serial.println("SD card is locked");
    return false;
  }
  return true;
}

bool initialized;

void setup() {
  pinMode(PS_ON_PIN,OUTPUT); digitalWrite(PS_ON_PIN,LOW);
  delay(2000);
  // Setup SD card pins
  Serial.begin(38400);
}

void write1K(uint32_t value) {
  for( int i = 1024; i > 0; i = i - 4) {
    sdcard.write_file(file, (uint8_t*)&value, 4);
  }
}

// return the number of failed bytes
int check1K(uint32_t value) {
  uint32_t read_value;
  int failures = 0;
  for( int i = 1024; i > 0; i = i - 4) {
    int16_t read_bytes = fat_read_file(file, (uint8_t*)&read_value, 4);
    uint8_t* value_ptr = (uint8_t*)&value;
    uint8_t* read_ptr = (uint8_t*)&read_value;
    for (int j = 0; j < 4; j++) {
      if (*value_ptr != *read_ptr) {
	failures++;
      }
      value_ptr++;
      read_ptr++;
    }
  }
  return failures;
}

void testA(long kilobytes) {
  if (!open_sd()) return;
  if (!create_slam_file("slamville.out")) return;
  if (!open_slam_file("slamville.out")) return;
  Serial.println("Beginning write...");
  long start = millis();
  // Write 1K
  for (long i = 0; i < kilobytes; i++) {
    write1K(i);
  }
  close_slam_file();
  Serial.print("Write completed in ");
  Serial.print(millis() - start, DEC);
  Serial.println(" ms.");
  long write_failures = 0;
  long read_failures = 0;
  for (int j = 0; j < 8; j++) {
    Serial.print("Beginning read cycle ");
    Serial.println(j, DEC);
    if (!open_slam_file("slamville.out")) {
      Serial.println("Couldn't open slamfile.");
      continue;
    }
    start = millis();
    for (long i = 0; i < kilobytes; i++) {
      read_failures += check1K(i);
    }
    close_slam_file();
    Serial.print("Read completed in ");
    Serial.print(millis()-start, DEC);
    Serial.print(", error count ");
    Serial.println(read_failures);
  }
}
  
  
void loop() {
    Serial.println("Ready to run SD slammer.");
    Serial.print("Enter number of K to slam: ");
    bool waiting_for_input = true;
    bool valid = true;
    long kilobytes = 0;
    while (waiting_for_input) {
      int input = Serial.read();
      if (input != -1) {
        Serial.print(input,BYTE);
	if (input == '\n' || input == '\r') { 
	  waiting_for_input = false;
	} else if (input >= '0' && input <= '9') {
	  kilobytes *= 10;
	  kilobytes += (input - '0');
	} else {
	  valid = false;
	}
      }
      delay(1);
    }
    // kill buffer
    Serial.println();
    while (Serial.available()) { Serial.read(); }
    if (!valid || kilobytes == 0) {
      Serial.println("Invalid value; try again.\n");
    } else {
      testA(kilobytes);
    }
}
