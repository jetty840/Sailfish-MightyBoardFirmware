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
 #include "EepromMap.hh"
#include "Eeprom.hh"
#include <avr/eeprom.h>

 
 
namespace RGB_LED{
	const static int LEDAddress = 0B11000100;
	uint8_t LEDSelect = 0;
	uint8_t blinkRate = 0;
 
void init(){
	 
	 TWI_init();
	 
	 
	 
	 // make sure out drive is correct
	 // verify pin out - RGB, i2c pins
	 
	 int error;
	 // set operation modes
	 uint8_t data1[2] = {LED_REG_MODE1, LED_ALL_CALL_ADDR};
	 error = TWI_write_data(LEDAddress, data1, 2);
	 
	 _delay_us(50);
	 
	 // output logic state inverted
	 // leds are configured with a totem pole structure
	 uint8_t data[2] = {LED_REG_MODE2, LED_OUT_INVERTED | LED_OUT_DRIVE};
	 error = TWI_write_data(LEDAddress, data, 2);

	_delay_us(50);
	 
	 
	 uint8_t data2[2] = {LED_REG_LEDOUT, LED_INDIVIDUAL};// & ( LED_RED | LED_GREEN | LED_BLUE)};
	 error = TWI_write_data(LEDAddress, data2, 2);
	 
	 _delay_us(50);
	 
	 	 
	 setDefaultColor();
	 
 }
 
void errorSequence(){
	
	setColor(255, 0, 0);
	setLEDBlink(12);
   
}

void setDefaultColor(){

	 // set frequency to slowest and duty cyle to zero (off)
	 uint8_t LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, 1);
	 uint32_t CustomColor = eeprom::getEeprom32(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::CUSTOM_COLOR_OFFSET, 0xFFFFFFFF);
	
	// blink rate has to be set first in order for color to register,
	// so set blink before each color
	 
	 switch(LEDColor){
		 case LED_DEFAULT_WHITE:
			setColor(255, 255, 255);
			break;
		 case LED_DEFAULT_BLUE:
			setColor(0, 0, 255);
			break;
		 case LED_DEFAULT_RED: 
		    setColor(255, 0, 0);
			break;
		 case LED_DEFAULT_GREEN: 
			setColor(0, 255, 0);
			break;
		 case LED_DEFAULT_ORANGE:	
			setColor(200, 50, 0);	
			break;
		 case LED_DEFAULT_PINK:
			setColor(70, 0, 70);
			break;
		 case LED_DEFAULT_PURPLE:
			setColor(200, 0, 200);
			break;
		 case LED_DEFAULT_CUSTOM:
			setColor(CustomColor >> 24, CustomColor >> 16, CustomColor >> 8);
			break;
		 case LED_DEFAULT_OFF:
			setColor(0, 0, 0);
			break;
	 }
}

void setLEDBlink(uint8_t rate){
	
	if(rate > 0){	
	
	 // turn group blink on
	 uint8_t data[2] = {LED_REG_MODE2, LED_OUT_INVERTED | LED_OUT_DRIVE | LED_GROUP_BLINK};
	 TWI_write_data(LEDAddress, data, 2);
	 
	 _delay_us(50);
	 
	 uint8_t data2[2] = {LED_REG_LEDOUT, LED_GROUP & ( LED_RED | LED_GREEN | LED_BLUE)};
	 TWI_write_data(LEDAddress, data2, 2);
	 
	 _delay_us(50);
	 
	 // set group blink rate
	 uint8_t data1[2] = {LED_REG_GRPPWM, 128};
	 TWI_write_data(LEDAddress, data1, 2);
	 
	 _delay_us(50);
	 
	 //set dimming frequency to zero
	 uint8_t data3[2] = {LED_REG_GRPFREQ, rate};
	 TWI_write_data(LEDAddress, data3, 2);
	}
	else{ 
	// turn group blink off
	 uint8_t data[2] = {LED_REG_MODE2, LED_OUT_INVERTED | LED_OUT_DRIVE };
	 TWI_write_data(LEDAddress, data, 2);
	 
	  _delay_us(50);
	 
	 uint8_t data2[2] = {LED_REG_LEDOUT, LED_INDIVIDUAL & ( LED_RED | LED_GREEN | LED_BLUE)};
	 TWI_write_data(LEDAddress, data2, 2);
	 
	  _delay_us(50);
	  
	 // set blink rate to zero
	 uint8_t data1[2] = {LED_REG_GRPPWM, rate};
	 TWI_write_data(LEDAddress, data1, 2);
	 
	  _delay_us(50);
	 
	 //set dimming frequency to zero
	 uint8_t data3[2] = {LED_REG_GRPFREQ, rate};
	 TWI_write_data(LEDAddress, data3, 2);
	 
	  _delay_us(50);
	 
	 setDefaultColor();
	}
	 
}

    // set LED color and store to EEPROM "custom" color area
void setCustomColor(uint8_t red, uint8_t green, uint8_t blue){
	eeprom::setCustomColor(red, green, blue);
	setColor(red, green, blue, true);
} 


void setColor(uint8_t red, uint8_t green, uint8_t blue, bool clearOld){

		
	 // set red
	 uint8_t data[2] = {LED_REG_PWM_RED, red};
	 TWI_write_data(LEDAddress, data, 2);
	 
	 _delay_us(50);
	 
	 // set red
	 uint8_t data1[2] = {LED_REG_PWM_GREEN, green};
	 TWI_write_data(LEDAddress, data1, 2);
	 
	 _delay_us(50);
	 
	 // set red
	 uint8_t data2[2] = {LED_REG_PWM_BLUE, blue};
	 TWI_write_data(LEDAddress, data2, 2);
}
}
