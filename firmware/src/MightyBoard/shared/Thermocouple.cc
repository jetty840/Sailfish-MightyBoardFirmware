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



#include "Thermocouple.hh"

enum therm_states{
	CHANNEL_0,
	CHANNEL_1,
	COLD_TEMP
};


// We'll throw in nops to get the timing right (if necessary)
inline void nop() {
        asm volatile("nop"::);
        asm volatile("nop"::);
        asm volatile("nop"::);
        asm volatile("nop"::);
        asm volatile("nop"::);
}


Thermocouple::Thermocouple(const Pin& do_p,const Pin& sck_p,const Pin& di_p, const Pin& cs_p, uint8_t pin_id) :
        do_pin(do_p),
        sck_pin(sck_p),
        di_pin(di_p),
        cs_pin(cs_p),
        channel_id(pin_id)
{
	current_temp = 0;
	cold_temp = 0;
	config_state = COLD_TEMP;
	read_state = CHANNEL_0;
}

uint16_t bit_reverse(uint16_t int_in){
	
	uint16_t rev_bit = 0;
	rev_bit |= (int_in & 0x01) << 15;
	rev_bit |= (int_in & 0x02) << 13;
	rev_bit |= (int_in & 0x04) << 11;
	rev_bit |= (int_in & 0x08) << 9;
	rev_bit |= (int_in & 0x10) << 7;
	rev_bit |= (int_in & 0x20) << 5;
	rev_bit |= (int_in & 0x40) << 3;
	rev_bit |= (int_in & 0x80) << 1;
	rev_bit |= (int_in & 0x100) >> 1;
	rev_bit |= (int_in & 0x200) >> 3;
	rev_bit |= (int_in & 0x400) >> 5;
	rev_bit |= (int_in & 0x800) >> 7;
	rev_bit |= (int_in & 0x1000) >> 9;
	rev_bit |= (int_in & 0x2000) >> 11;
	rev_bit |= (int_in & 0x4000) >> 13;
	rev_bit |= (int_in & 0x8000) >> 15;
	
	return rev_bit;
}

uint16_t Thermocouple::get_current_temperature(uint8_t channel){
	
	return current_temp[channel];
}

void Thermocouple::init() {
	do_pin.setDirection(true);
	sck_pin.setDirection(true);
	di_pin.setDirection(false);
	cs_pin.setDirection(true);
	
	channel_0_config =  bit_reverse(INPUT_CHAN_23 | AMP_2_04 | WRITE_CONFIG); // reverse order for shifting out MSB first
	channel_1_config =  bit_reverse(INPUT_CHAN_23 | AMP_2_04 | WRITE_CONFIG);
	cold_temp_config = bit_reverse(TEMP_SENSE_MODE | WRITE_CONFIG);
	
	current_temp[0] = 0; current_temp[1] = 0; current_temp[2] = 0;
	cold_temp = 0;

	cs_pin.setValue(false);   // chip select hold low
	sck_pin.setValue(false);  // Clock select is active low
}



Thermocouple::SensorState Thermocouple::update() {

	sck_pin.setValue(false);
	
	// check that data ready flag is low
	// this pin actually pulses when data is ready, but we're not tracking the pulse
	// there is a high state after setting the config register when data is not ready
	// this is the state we are avoiding.  
	if(!di_pin.getValue())
		return;
		
	// the config register determines the output for the next read
	switch ( config_state){
		case CHANNEL_0 : 
			config = channel_0_config; 
			break;
		case CHANNEL_1 : 
			config = channel_1_config; 
			break;
		case COLD_TEMP : 
			config = cold_temp_config; 
			break;
	}
	
	uint16_t config_reg = 0;
	uint16_t raw = 0;
	
	bool bad_temperature = false; // Indicate a disconnected state
	
	// read the temperature register
	for (int i = 0; i < 16; i++) {
		
		// shift out config register data
		do_pin.setValue((config & 0b01) != 0);
		config >>= 1;
		
		sck_pin.setValue(true);
		raw = raw << 1;
		if (di_pin.getValue()) {raw = raw | 0x01; }

		sck_pin.setValue(false);
	}
	
	// read back the config reg
	for (int i = 0; i < 16; i++) {
		
		// shift out dummy data
		do_pin.setValue(false);
		
		sck_pin.setValue(true);
		config_reg = config_reg << 1;
		if (di_pin.getValue()) { config_reg = config_reg | 0x01; }

		sck_pin.setValue(false);
	}
	
	sck_pin.setValue(false);

	if (bad_temperature) {
	  // Set the temperature to 1024 as an error condition
	  current_temp[read_state] = BAD_TEMPERATURE;
	  return SS_ERROR_UNPLUGGED;
	}

	current_temp[read_state] = raw;
	if (read_state == COLD_TEMP){
		cold_temp = raw;
	}
	
	// the temp read is determined by the config state just sent
	read_state = config_state;
	
	// the config register determines the output for the next read
	switch ( config_state){
		case CHANNEL_0 : 
			config_state = CHANNNEL_1; 
			break;
		case CHANNEL_1 : 
			// we don't need to read the cold temp every time
			// read it ~once per minute
			temp_check_counter++;
			if(temp_check_counter == TEMP_CHECK_COUNT){
				temp_check_counter = 0;
				config_state = COLD_TEMP;  
			}else{
				config_state = CHANNEL_O;}
			break;
		case COLD_TEMP : 
			config_state = CHANNEL_0; 
			break;
	}
	
	return SS_OK;
}
