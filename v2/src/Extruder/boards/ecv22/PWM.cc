/*
 * PWM.cc
 *
 *  Created on: Feb 23, 2010
 *      Author: phooky
 */

#include "PWM.hh"

class PWMTableEntry {
public:
	Pin pin_;
	uint8_t value_;
	PWMTableEntry(Pin& pin, uint8_t value) : pin_(pin), value_(value) {}
}

#define MAX_ENTRIES 8

uint8_t tab_storage[sizeof(PWMTableEntry)*MAX_ENTRIES];

PWMTableEntry* table = tab_storage;
int8_t last_entry = 0;


inline void doCycle(uint8_t val) {
	for (int8_t i = 0; i < last_entry; i++) {
		table[i].pin_.setValue(table[i].value_ <= val);
	}
}

void startPWM() {

}

void setPWM(Pin& pin, uint8_t val) {
	for (int8_t i = 0; i < last_entry; i++) {
		if (table[i].pin_ == pin) {
			if (val == 0 || val == 0xff) {
				table[i] = table[last_entry-1];
				last_entry--;
				pin.setValue(val == 0xff);
			} else {
				table[i].value_ = val;
			}
			return;
		}
	}
	table[last_entry++] = PWMTableEntry(pin,val);
}
