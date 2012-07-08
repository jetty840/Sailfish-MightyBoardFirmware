/*
 * Copyright 2012 by Alison Leonard alison@makerbot.com
 * 
 * @file ThermocoupleReader.cc
 * @brief This class reads temperature data from the ADS1118 ADC from Texas Instruments
 * The ADC has two thermocouple input channels and a cold junction temperature sensor
 * 
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
#include "TemperatureTable.hh"


/*
 * Thermocouple Reader Constructor
 * Create a new thermocouple instance, and attach it to the given pins.
 * 
 * @param [in] do_p Data Out: MOSI (output).
 * @param [in] sck_p Clock Pin (output). Can be shared with other thermocouples.
 * @param [in] di_p Data In: MISO (input)
 * @param [in] cs_p Chip Select (output)
 */
ThermocoupleReader::ThermocoupleReader(const Pin& do_p,const Pin& sck_p,const Pin& di_p, const Pin& cs_p) :
        do_pin(do_p),
        sck_pin(sck_p),
        di_pin(di_p),
        cs_pin(cs_p)
{
	
}
/*
 * Reverse bit order of a uint16_t to match required format for SPI communcation
 * 
 * @param[in] 	int_in uint16_t to be reversed
 * @return[out] bit reversed uint16_t
 * 
 */
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


/*
 * Initialize ThermocoupleReader pins and set read variables to default state
 * 
 */
void ThermocoupleReader::init() {
	
	do_pin.setDirection(true);
	sck_pin.setDirection(true);
	di_pin.setDirection(false);
	cs_pin.setDirection(true);
	
	channel_one_config =  bit_reverse(INPUT_CHAN_01 | AMP_0_256 | SAMPLE_FREQ_64 | WRITE_CONFIG); // reverse order for shifting out MSB first
	channel_two_config =  bit_reverse(INPUT_CHAN_23 | AMP_0_256 | SAMPLE_FREQ_64 | WRITE_CONFIG);
	cold_temp_config = bit_reverse(TEMP_SENSE_MODE | SAMPLE_FREQ_64 | WRITE_CONFIG);
	
	channel_one_temp = 0;
	channel_two_temp = 0;
	cold_temp = 0;
	
	cs_pin.setValue(false);   // chip select hold low
	sck_pin.setValue(false);  // Clock select is active low
	
	last_temp_updated = NULL;
	
	initConfig();
}

/*
 * Send initial config value to the ADS1118
 * 
 */
void ThermocoupleReader::initConfig(){

	sck_pin.setValue(false);
	
	
	config_state = CHANNEL_ONE;
	read_state = CHANNEL_ONE;
	temp_check_counter = TEMP_CHECK_COUNT;
	
	uint16_t config = channel_one_config;
	
	uint16_t config_reg = 0;
	
	// send the config register 
	for (int i = 0; i < 16; i++) {
		
		// shift out config register data
		do_pin.setValue((config & 0b01) != 0);
		config >>= 1;
		
		sck_pin.setValue(true);
		// we don't care about the slave data here
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
	
	/// we could check here to make sure the config data has been read correctly
	
	sck_pin.setValue(false);

}


/*
 * Get temperature read
 * 
 * @param [in] channel  which ADC channel are we reading (valid channels: 1,0)
 * @return last temperature reading for channel
 * 
 */
int16_t ThermocoupleReader::GetChannelTemperature(uint8_t channel){
	
	if (channel == CHANNEL_ONE){
		return channel_one_temp;
	}else{

		return channel_two_temp;
	}

}


/*
 * Get a new read from the ADC.  This function is called by the motherboard slice at regular intervals
 * and cycles between channel 1 channel 2 and cold junction temperature, reading one value each fucntion call
 * 
 */
bool ThermocoupleReader::update() {

	sck_pin.setValue(false);
	
	// check that data ready flag is low
	// if it is high, return false so the calling function knows to try again
	if(di_pin.getValue())
		return false;
		
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
	
	/// the ADS1118 uses bidirection SPI communication
	/// the sensor returns 4 bytes of data per read.  the first two bytes are the 
	/// ADC bits.  the second two bytes are the config register bits
	/// the mightyboard (master) sends the desired configuration register in the first 
	/// two bytes and sends dummy data for the second two bytes
	
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

	int16_t temp;
	/// store read to the temperature variable
	switch(read_state){
		case COLD_TEMP:
			cold_temp = TemperatureTable::TempReadtoCelsius((int16_t)(raw >> 2), TemperatureTable::table_cold_junction, MAX_TEMP);
			break;
		case CHANNEL_ONE:
			if (raw == UNPLUGGED_TEMPERATURE){
				channel_one_temp = UNPLUGGED_TEMPERATURE;
			}else{
				temp = TemperatureTable::TempReadtoCelsius((int16_t)raw, TemperatureTable::table_thermocouple, MAX_TEMP);
				if (temp != MAX_TEMP){
					channel_one_temp = temp + cold_temp;
				/// MAX_TEMP is a flagged temperature we look for in ThermocoupleDual.cc, the handler for the heater class 
				}else{
					channel_two_temp = MAX_TEMP;
				}
			}
			break;
		case CHANNEL_TWO:
			if (raw == UNPLUGGED_TEMPERATURE){
				channel_two_temp = UNPLUGGED_TEMPERATURE;
			}else{
				temp = TemperatureTable::TempReadtoCelsius((int16_t)raw, TemperatureTable::table_thermocouple, MAX_TEMP);
				if (temp != MAX_TEMP){
					channel_two_temp = temp + cold_temp;
				/// MAX_TEMP is a flagged temperature we look for in ThermocoupleDual.cc, the handler for the heater class 
				}else{
					channel_two_temp = MAX_TEMP;
				}
			}
			break;

	}
	
	/// track last update temperature, so that this value can be queried.
	last_temp_updated = read_state;
	/// the temperature read next cycle is determined by the config bytes we just sent
	read_state = config_state;
	
	/// update the config register
	/// we switch back and forth between channel one and channel two
	/// every TEMP_CHECK_COUNT cycles, we read the cold_junction_temperature
	switch ( config_state){
		case CHANNEL_ONE : 
			config_state = CHANNEL_TWO; 
			break;
		case CHANNEL_TWO : 
			// we don't need to read the cold temp every time
			// read it ~once per minute
			temp_check_counter++;
			if(temp_check_counter >= TEMP_CHECK_COUNT){
				temp_check_counter = 0;
				config_state = COLD_TEMP;  
			}else{
				config_state = CHANNEL_ONE;
				}
			break;
		case COLD_TEMP : 
			config_state = CHANNEL_ONE; 
			break;
	}	
	
	// return true when temperature update is successful
	return true;
}
