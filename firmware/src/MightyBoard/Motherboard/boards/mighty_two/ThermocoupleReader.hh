/*
 * Copyright 2012 by Alison Leonard alison@makerbot.com
 * 
 * @file ThermocoupleReader.hh
 * @brief This class reads temperature data from the ADS1118 ADC from Texas Instruments
 * The ADC has two thermocouple input channels and a cold junction temperature sensor
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

#ifndef THERMOCOUPLE_READER_HH_
#define THERMOCOUPLE_READER_HH_

#include "Pin.hh"

/// Configuration Register Maps for the ADS1118 ADC

/// Channel select bits 
#define INPUT_CHAN_01 	0x0000
#define INPUT_CHAN_23 	0x3000

/// ADC gain select bits
/// the gain setting sets the voltage range for the ADC.  Full Scale
/// voltage is the read value at 0x7FFF  (the ADC returns a 16bit integer integer value)
#define AMP_6_14		0x0000  ///Full Scale Voltage is 6.14V
#define AMP_4_09		0x0200  ///Full Scale Voltage is 4.09V
#define AMP_2_04		0x0400  ///Full Scale Voltage is 2.04V  
#define AMP_1_02		0x0300  ///Full Scale Voltage is 1.02V
#define AMP_0_512		0x0800  ///Full Scale Voltage is 0.512V
/// we use the smallest gain setting - k-Type thermocouples have a voltage 
/// difference of ~12mV at 300C
#define AMP_0_256		0x0C00  ///Full Scale Voltage is 0.256V  

/// Sample Frequency select bits (Hz)
#define SAMPLE_FREQ_128 0x0080	
#define SAMPLE_FREQ_64	0x0060	
#define	SAMPLE_FREQ_32  0x0040
#define SAMPLE_FREQ_64  0x0020

/// single sample vs continous conversion
#define SINGLE_MODE		0x0100

/// ADC mode (thermocouples) vs temperature sensor (on-board cold_junction temp sensor)
#define TEMP_SENSE_MODE 0x0010

/// write new data to the config register ( if bits <2:1> are not <01> the config bytes are ignored)
#define WRITE_CONFIG	0x0002

/// number of read cycles between cold junction temperature reads
/// we don't need to read the cold junction temperature every cycle 
/// because we don't expect it to change much
#define TEMP_CHECK_COUNT 120

/// The thermocouple module provides a bitbanging driver that can read the
/// temperature from the ADS1118 sensor, and also report on any error conditions.
/// \ingroup SoftwareLibraries
class ThermocoupleReader {
	
public:
	enum therm_states{
		CHANNEL_ONE = 0,
		CHANNEL_TWO = 1,
		COLD_TEMP = 2
	};

private:
        Pin cs_pin;  ///< Chip select pin (output)
        Pin sck_pin; ///< Clock pin (output)
        Pin do_pin;  ///< Data out pin (output)
        Pin di_pin;  ///< Data in pin (output)
        
        /// 
        uint8_t config_state;
        uint8_t read_state;
        uint8_t temp_check_counter;
        
        int16_t cold_temp;
        uint16_t channel_one_temp;
        uint16_t channel_two_temp;  
        
        uint16_t channel_one_config; 	// config register settings to read thermocouple data
        uint16_t channel_two_config; 	// config register settings to read thermocouple data
        uint16_t cold_temp_config; 		// config register settings to read cold junction temperature
        
        uint16_t last_temp_updated;
      
        
public:
        /// Create a new thermocouple instance, and attach it to the given pins.
        /// \param [in] do_p Data Out: MOSI (output).
        /// \param [in] sck_p Clock Pin (output). Can be shared with other thermocouples.
        /// \param [in] di_p Data In: MISO (input)
        /// \param [in] cs_p Chip Select (output)
	ThermocoupleReader(const Pin& do_p,const Pin& sck_p,const Pin& di_p, const Pin& cs_p);

	void init();
	void initConfig();
	
	bool update();
	
	uint8_t getLastUpdated(){ return last_temp_updated;}
	
	int16_t GetChannelTemperature(uint8_t channel);
	
	int16_t get_cold_temperature() {return cold_temp;}
	
	///  if no thermocouple is plugged in, the ADC returns 0x7fff (FullScale)
	///  temperature at 300C is ~0x062f, ie not at all close to full scale
	const static int16_t UNPLUGGED_TEMPERATURE = 0x7fff;
	
	/// safety value if ADC read is out of range
    const static int16_t MAX_TEMP = 400;
};
#endif // THERMOCOUPLE_READER_HH_
