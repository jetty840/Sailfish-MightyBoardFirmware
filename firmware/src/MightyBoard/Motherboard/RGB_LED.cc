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
	uint8_t LEDSelect = 0;
 
void init(){
	 
	 TWI_init();
	 
	 // set frequency to slowest and duty cyle to zero (off)
	 uint8_t data4[4] = {LED_REG_PSC0, 0, LED_REG_PWM0, 0};
	 uint8_t error = TWI_write_data(LEDAddress, data4, 4);
	 
	 // set all outputs to PWM0
	 uint8_t data6[6] = {LED_REG_SELECT, LED_BLINK_PWM0};
	 LEDSelect = LED_BLINK_PWM0;
	 					
	 error = TWI_write_data(LEDAddress, data6, 6);
 }
 // channel : 1,2 select PWM channels, 3 is a pure on / off channel
 // level : duty cycle (brightness) for channels 1,2,  
 //			for Channel 3, level is on if not zero
 // LEDs:  {bits: XXBBGGRR : BLUE: 0b110000, Green:0b1100, RED:0b11} 
 //  		ones indicate on, zeros indicate off 
 void setBrightness(uint8_t Channel, uint8_t level, uint8_t LEDs)
 {
 	uint8_t data[4] = {0 , level, LED_REG_SELECT, 0};
 	
	// set pwm for select channel
 	if (Channel == LED_CHANNEL1){
 		data[0] = LED_REG_PWM0;
 		// clear past select data and apply PWM0
 		data[3] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM0 & LEDs);
 	}
 	else if (Channel == LED_CHANNEL2){
 		data[0] = LED_REG_PWM1;
 		// clear past select data and apply PWM1
 		data[3] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM1 & LEDs);
 	}
 	else {
 		toggleLEDNoPWM((level != 0), LEDs);
 		return;
 	}
 	
 	uint8_t error = TWI_write_data(LEDAddress, data, 4);
 	LEDSelect = data[3];
 		
 }
  // channel : 1,2 select PWM channels, channel 3 does nothing
 // level : blink rate for channels 1,2,  
 // LEDs:  {bits: XXBBGGRR : BLUE: 0b110000, Green:0b1100, RED:0b11} 
 //  		ones indicate on, zeros indicate off 
 void setBlinkRate(uint8_t Channel, uint8_t rate, uint8_t LEDs)
 {
 	uint8_t data[4] = {0 , rate, LED_REG_SELECT, 0};
 	
 	// set pwm for select channel
 	if (Channel == LED_CHANNEL1){
 		data[0] = LED_REG_PSC0;
 		// clear past select data and apply PWM0
 		data[3] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM0 & LEDs);
 	}
 	else if (Channel == LED_CHANNEL2){
 		data[0] = LED_REG_PSC1;
 		// clear past select data and apply PWM1
 		data[3] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM1 & LEDs);
 	}
 	else
 		return;
 	
 	uint8_t error = TWI_write_data(LEDAddress, data, 4);
 	LEDSelect = data[3];	
 }
 
 // channel 3 sets LEDs on or off 
 // LEDs:  {bits: XXBBGGRR : BLUE: 0b110000, Green:0b1100, RED:0b11} 
 //  		ones indicate on, zeros indicate off 
 void toggleLEDNoPWM(bool enable, uint8_t LEDs)
 {
 	uint8_t data[2] = {LED_REG_SELECT, 0};
 	
 	if(enable)
 	// clear past select data and turn LEDs full on
 		data[1] = (LEDSelect & ~LEDs) | (LED_ON & LEDs);
 	else
 	// clear past select data and turn LEDs full off
 		data[1] = (LEDSelect & ~LEDs) | (LED_OFF & LEDs); 
 		
 	uint8_t error = TWI_write_data(LEDAddress, data, 2);
 	LEDSelect = data[1];
 }
 
 void startupSequence(){
	 
	 bool toggle = false;
	 uint8_t data[4] = {LED_REG_PSC0, 0, LED_REG_SELECT, LED_BLINK_PWM0 << LED_RED_BITS};  
	 uint8_t error = TWI_write_data(LEDAddress, data, 4);
	 
	 data[0] = LED_REG_PWM0; // duty reg
	 for(int i =0; i < 60; i++)
	 {
	 	_delay_us(50);
		data[1] = i;
		error = TWI_write_data(LEDAddress, data, 2);			
	}
	for(int i = 60; i >= 0; i--)
	{
		_delay_us(50);
		data[1] = i;
		TWI_write_data(LEDAddress, data, 2);
	}
	 
 }
 
void errorSequence(){
}
}
