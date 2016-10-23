/*
 * This class reads temperature data from an AD8495 thermocouple amp
 * as well as the thermistor readings from the onchip 10bit ADC
 *
 *   Vref = 0.0V  [Cannot sense temps below 0C]
 *   ADC = reported 10bit A/D value
 *   Vsense = 5.0V * (ADC / 1024)
 *
 *   Temp = (Vsense - Vref) / 5 mV/C
 *        = Vsense * 1C / 5mV * ( 1000 mV / 1V )
 *        = Vsense * 200C / V
 *        = 5.0V * ADC * 200C / 1024 V
 *        = ADC * 1000 / 1024 C
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

#include "Compat.hh"
#include "Configuration.hh"
#include "AnalogPin.hh"
#include "ThermocoupleReader.hh"
#include "TemperatureTable.hh"
#include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>

/*
 * Thermocouple Reader Constructor
 * Create a new thermocouple instance, and attach it to an ADC pin.
 * 
 * @param [in] pinA -- Extruder A ADC pin number (0 - 15)
 * @param [in] pinB -- Extruder B ADC pin number (0 - 15)
 * @param [in] pinH -- HBP ADC pin number (0 - 15)
 */
ThermocoupleReader::ThermocoupleReader() :
     error_code(TemperatureSensor::SS_OK),
     last(THERM_CHANNEL_TWO),
     pindex(THERM_CHANNEL_HBP)
{
     reset();
}

void ThermocoupleReader::reset() {

	 // Values will be put into range by getThermistorTable()
	 for (uint8_t i = 0; i < TEMP_NSENSORS; i++)
		 table_indices[i] = eeprom::getThermistorTable(i);

	 // Impose restrictions

	 if ( table_indices[THERM_CHANNEL_HBP] == TABLE_THERMOCOUPLE_K )
		 table_indices[THERM_CHANNEL_HBP] = TABLE_HBP_THERMISTOR;

     // Get the sensor types
     sensor_types = 0;  // Assume all thermistors

	 for (uint8_t i = 0; i < TEMP_NSENSORS; i++)
		 if ( table_indices[i] == TABLE_THERMOCOUPLE_K )
			 sensor_types |= ( 1 << i );

	 // Pin assignments

	 // We forced the HBP to be on a thermistor....
     pin[THERM_CHANNEL_HBP] = HBP_THERMISTOR_PIN;

     // Determine which ADC pins to use for the extruders

     if ( sensor_types & ( 1 << THERM_CHANNEL_ONE ) ) {
		 pin[THERM_CHANNEL_ONE] = EXA_THERMOCOUPLE_PIN;

		 // Set unused ADC pin as output and set it LOW
		 EXA_THERMISTOR_DDR  |=   EXA_THERMISTOR_MASK;
		 EXA_THERMISTOR_PORT &= ~(EXA_THERMISTOR_MASK);
     }
     else {
		 pin[THERM_CHANNEL_ONE] = EXA_THERMISTOR_PIN;

		 // Set unused ADC pin as output and set it LOW
		 EXA_THERMOCOUPLE_DDR  |=   EXA_THERMOCOUPLE_MASK;
		 EXA_THERMOCOUPLE_PORT &= ~(EXA_THERMOCOUPLE_MASK);
     }

     if ( sensor_types & ( 1 << THERM_CHANNEL_TWO ) ) {
		 pin[THERM_CHANNEL_TWO] = EXB_THERMOCOUPLE_PIN;

		 // Set unused ADC pin as output and set it LOW
		 EXB_THERMISTOR_DDR  |=   EXB_THERMISTOR_MASK;
		 EXB_THERMISTOR_PORT &= ~(EXB_THERMISTOR_MASK);
     }
     else {
		 pin[THERM_CHANNEL_TWO] = EXB_THERMISTOR_PIN;

		 // Set unused ADC pin as output and set it LOW
		 EXB_THERMOCOUPLE_DDR  |=   EXB_THERMOCOUPLE_MASK;
		 EXB_THERMOCOUPLE_PORT &= ~(EXB_THERMOCOUPLE_MASK);
     }

     // And quiesce the other unused AD ports
     ADC_UNUSED_DDR1  |=   ADC_UNUSED_MASK1;
     ADC_UNUSED_PORT1 &= ~(ADC_UNUSED_MASK1);

     ADC_UNUSED_DDR2  |=   ADC_UNUSED_MASK2;
     ADC_UNUSED_PORT2 &= ~(ADC_UNUSED_MASK2);

     cnt   = 0;
     accum = 0;

     for (uint8_t i = 0; i < TEMP_NSENSORS; i++)
	  temp[i] = 0.0;
}

void ThermocoupleReader::init() {
     reset();

     for (uint8_t i = 0; i < TEMP_NSENSORS; i++)
		 initAnalogPin(pin[i]);

     // Initiate the first ADC sample
     finished = false;
     startAnalogRead(pin[pindex], &raw, &finished);
}

/*
 * Get temperature read
 * 
 * @param [in] channel -- Sensor we reading (0, 1, ..., TEMP_NSENSORS)
 * @return last temperature reading for channel
 * 
 */
TemperatureSensor::SensorState ThermocoupleReader::GetChannelTemperature(uint8_t channel,
									 volatile float &read_temperature)
{
     read_temperature = temp[channel];
     return error_code;
}

/*
 * Get a new read from the ADC.
 * This function is called by the motherboard slice at regular intervals
 * Tool 0, Tool 1, and the HBP.
 */
uint8_t ThermocoupleReader::update() {
     uint8_t retval;
     bool valid;
     static uint8_t attempts = 0;

#define NEXT_CHANNEL				\
     last = pindex;					\
     accum = 0;						\
     cnt = 0;						\
     if ( ++pindex > THERM_CHANNEL_HBP ) pindex = 0

     ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
	  valid  = finished;
     }

     // Process prior ADC sample
     if ( !valid ) {
	  // ADC not completed yet
	  error_code = TemperatureSensor::SS_ADC_BUSY;
	  if ( ++attempts < 10 ) return THERM_ADC_BUSY;

	  // Stubborn channel; move on to the next one
	  NEXT_CHANNEL;
	  retval = THERM_ADC_BUSY;
	  goto retry;
     }

     // ADC completed; process the result

     // Accumulate the value
     accum += raw;

     // Once we have TEMP_OVERSAMPLE values, compute the corresponding temperature

     if ( ++cnt >= TEMP_OVERSAMPLE ) {

	  // Sensor type: thermocouple (1) or thermistor (0)

	  if ( sensor_types & (1 << pindex) ) {

	       // Thermocouple ADC
	       // We can handle this faster than using a table
	       if ( !(ADC_THERMOCOUPLE_DISCONNECTED(accum)) ) {
			   temp[pindex] = (float)((int32_t)accum * 1000L)/(float)(1024 * TEMP_OVERSAMPLE);
			   error_code = (temp[pindex] < MAX_TEMP) ?
				   TemperatureSensor::SS_OK : TemperatureSensor::SS_BAD_READ;
	       }
	       else {
			   // Value appears suspect; signal an error indicating that the
			   // sensor is disconnected
			   temp[pindex] = MAX_TEMP;
			   error_code = TemperatureSensor::SS_ERROR_UNPLUGGED;
	       }
	  }
	  else {

	       // Thermistor ADC
	       if ( !(ADC_THERMISTOR_DISCONNECTED(accum)) ) {
			   temp[pindex] = TemperatureTable::TempReadtoCelsius(
				   accum, table_indices[pindex],
				   MAX_TEMP);
			   error_code = (temp[pindex] < MAX_TEMP) ?
				   TemperatureSensor::SS_OK : TemperatureSensor::SS_BAD_READ;
	       }
	       else {
			   // Value appears suspect; signal an error indicating that the
			   // sensor is disconnected
			   error_code = TemperatureSensor::SS_ERROR_UNPLUGGED;
			   temp[pindex] = MAX_TEMP;
	       }
	  }

	  // Okay to process this temp sensor in the higher level code
	  retval = THERM_READY;    

	  // Move to the next sensor
	  NEXT_CHANNEL;
     }
     else
	  // Not yet time to process this sensor; needs to accumulate more AD readings
	  retval = THERM_NOT_READY;

retry:
     // Kick off the next ADC read
     attempts = 0;
     finished = false;
     startAnalogRead(pin[pindex], &raw, &finished);

     return retval;
}
