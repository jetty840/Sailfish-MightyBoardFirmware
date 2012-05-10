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

#ifndef THERMOCOUPLE_HH_
#define THERMOCOUPLE_HH_

#include "TemperatureSensor.hh"
#include "Pin.hh"

#define INPUT_CHAN_01 	0x0000
#define INPUT_CHAN_23 	0x3000
#define AMP_2_04		0x0400
#define AMP_4_09		0x0200
#define AMP_1_02		0x0300

#define SINGLE_MODE		0x0100

#define TEMP_SENSE_MODE 0x0010

#define WRITE_CONFIG	0x0002

/// The thermocouple module provides a bitbanging driver that can read the
/// temperature from (chip name) sensor, and also report on any error conditions.
/// \ingroup SoftwareLibraries
class Thermocouple : public TemperatureSensor {
private:
        Pin cs_pin;  ///< Chip select pin (output)
        Pin sck_pin; ///< Clock pin (output)
        Pin do_pin;  ///< Data out pin (output)
        Pin di_pin;  ///< Data in pin (output)
        
        uint8_t channel_id;
        
        uint16_t data_config; // config register settings to read thermocouple data
        uint16_t temp_config; // config register settings to read cold junction temperature
public:
        /// Create a new thermocouple instance, and attach it to the given pins.
        /// \param [in] cs Chip Select (output).
        /// \param [in] sck Clock Pin (output). Can be shared with other thermocouples.
        /// \param [in] so Data Pin (input)
	Thermocouple(const Pin& do_p,const Pin& sck_p,const Pin& di_p, const Pin& cs_p, uint8_t pid_id);
	
	void set_reference_temperature(uint16_t temp);
	
	void get_current_temperature(uint8_t channel);

	void init();

	SensorState update();
};
#endif // THERMOCOUPLE_HH_
