/*
 * Copyright 2011 by Alison Leonard	 <alison@makerbot.com>
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
 
 
 #include "RGB_LED.hh"
 #include "TWI.hh"
 #include <util/delay.h>
 #include "Configuration.hh"
 #include "Pin.hh"

 
 
namespace RGB_LED{
	const static int LEDAddress = 0B11000100;
 
void init(){
	 
	 TWI_init();
 }
 
 //TODO make definitions for led commands / registers
 void startupSequence(){
	 
	 uint8_t data[4] = {0B00000001, 0, 0B00000101, 0B00000010}; // freq reg, zero freq, led reg, freq/duty1 
	 uint8_t error = TWI_write_data(LEDAddress, data, 4);

	 data[0] = 0B00000010; // duty reg
	 for(int i =0; i < 60; i++)
	 {
	 	_delay_us(50000);
		data[1] = i;
		error = TWI_write_data(LEDAddress, data, 2);
/*		if(error)
	 {
		 for(uint8_t i = 0; i < error; i++)
			{
				DEBUG_PIN1.setValue(true);
				_delay_us(300000);
				DEBUG_PIN1.setValue(false);
				_delay_us(300000);
			}
	 }
	*/	
	}
	for(int i = 60; i >= 0; i--)
	{
		_delay_us(50000);
		data[1] = i;
		TWI_write_data(LEDAddress, data, 2);
		
	}
	 
 }
 
 void errorSequence(){
 }
}
 
