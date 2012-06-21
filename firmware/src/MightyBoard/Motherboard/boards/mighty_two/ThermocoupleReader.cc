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



#include "ThermocoupleReader.hh"
#include "stdio.h"
#include "Configuration.hh"

enum therm_states{
	CHANNEL_ONE,
	CHANNEL_TWO,
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


ThermocoupleReader::ThermocoupleReader(const Pin& do_p,const Pin& sck_p,const Pin& di_p, const Pin& cs_p) :
        do_pin(do_p),
        sck_pin(sck_p),
        di_pin(di_p),
        cs_pin(cs_p)
{
	
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

void ThermocoupleReader::init() {
	
	do_pin.setDirection(true);
	sck_pin.setDirection(true);
	di_pin.setDirection(false);
	cs_pin.setDirection(true);
	
	channel_one_config =  bit_reverse(INPUT_CHAN_01 | AMP_0_2 | WRITE_CONFIG); // reverse order for shifting out MSB first
	channel_two_config =  bit_reverse(INPUT_CHAN_23 | AMP_0_2 | WRITE_CONFIG);
	cold_temp_config = bit_reverse(TEMP_SENSE_MODE | WRITE_CONFIG);
	
	channel_one_temp = 0;
	channel_two_temp = 0;
	cold_temp = 0;
	
	config_state = CHANNEL_ONE;
	read_state = CHANNEL_ONE;
	temp_check_counter = 0;

	cs_pin.setValue(false);   // chip select hold low
	sck_pin.setValue(false);  // Clock select is active low

}


uint16_t ThermocoupleReader::GetChannelTemperature(uint8_t channel){
	
	if (channel == 0){
		return channel_one_temp;
	}else{

		return channel_two_temp;
	}

}

void ThermocoupleReader::update() {

	sck_pin.setValue(false);
	
	// check that data ready flag is low
	// this pin actually pulses when data is ready, but we're not tracking the pulse
	// there is a high state after setting the config register when data is not ready
	// this is the state we are avoiding.  
	//if(!di_pin.getValue())
	//	return;
		
	uint16_t config = 0;
		
	// the config register determines the output for the next read
	switch ( config_state){
		case CHANNEL_ONE : 
			config = channel_one_config; 
			break;
		case CHANNEL_TWO : 
			config = channel_two_config; 
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
	  if(read_state < 2){
		//current_temp[read_state] = BAD_TEMPERATURE;
		//temp_state[read_state] =  SS_ERROR_UNPLUGGED;
		}
		return; 
	}
	
	switch(read_state){
		case COLD_TEMP:
			cold_temp = raw >> 2;
			break;
		case CHANNEL_ONE:
			channel_one_temp = raw;
			break;
		case CHANNEL_TWO:
			channel_two_temp = raw;
			break;
	}
	
	//temp_state[read_state] = SS_OK;
	
	// the temp read is determined by the config state just sent
	read_state = config_state;
	
	// the config register determines the output for the next read
	switch ( config_state){
		case CHANNEL_ONE : 
			config_state = CHANNEL_TWO; 
			break;
		case CHANNEL_TWO : 
			// we don't need to read the cold temp every time
			// read it ~once per minute
		//	temp_check_counter++;
		//	if(temp_check_counter == TEMP_CHECK_COUNT){
		//		temp_check_counter = 0;
				config_state = COLD_TEMP;  
		//	}else{
		//		config_state = CHANNEL_0;
				//}
			break;
		case COLD_TEMP : 
			config_state = CHANNEL_ONE; 
			break;
	}
	

	
}
