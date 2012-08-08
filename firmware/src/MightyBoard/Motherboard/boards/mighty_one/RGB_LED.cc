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
	 
	 setDefaultColor();
 }
    
 // channel : 1,2 select PWM channels, 3 is a pure on / off channel
 // level : duty cycle (brightness) for channels 1,2,  
 //			for Channel 3, level is on if not zero
 // LEDs:  {bits: XXBBGGRR : BLUE: 0b110000, Red:0b1100, Green:0b11} 
 //  		ones indicate on, zeros indicate off 
 void setBrightness(uint8_t Channel, uint8_t level, uint8_t LEDs)
 {
 	uint8_t data[4] = {LED_REG_SELECT, 0, 0 , level};
     uint8_t data1[2] = {LED_REG_SELECT, 0};
     uint8_t data2[2] = {0, level};

	// set pwm for select channel
 	if (Channel == LED_CHANNEL1){
 		data2[0] = LED_REG_PWM0;
 		// clear past select data and apply PWM0
 		data1[1] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM0 & LEDs);
 	}
 	else if (Channel == LED_CHANNEL2){
 		data2[0] = LED_REG_PWM1;
 		// clear past select data and apply PWM1
 		data1[1] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM1 & LEDs);
 	}
 	else {
 		toggleLEDNoPWM((level != 0), LEDs);
 		return;
 	}
 	
 	uint8_t error = TWI_write_data(LEDAddress, data1, 2);
     _delay_us(1);
    error = TWI_write_data(LEDAddress, data2, 2);
     _delay_us(1);
 	
     LEDSelect = data1[1];
 		
 }
    
  // channel : 1,2 select PWM channels, channel 3 does nothing
 // level : blink rate for channels 1,2,  channel 3 ignores this
 // LEDs:  {bits: XXBBGGRR : BLUE: 0b110000, Green:0b1100, RED:0b11} 
 //  		ones indicate on, zeros indicate off 
 void setBlinkRate(uint8_t Channel, uint8_t rate, uint8_t LEDs)
 {
	 uint8_t data[4] = {0 , rate, LED_REG_SELECT, 0};
     uint8_t data1[2] = {LED_REG_SELECT, 0};
     uint8_t data2[2] = {0 , rate};
 	
 	// set pwm for select channel
 	if (Channel == LED_CHANNEL1){
 		data2[0] = LED_REG_PSC0;
 		// clear past select data and apply PWM0
 		data1[1] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM0 & LEDs);
 	}
 	else if (Channel == LED_CHANNEL2){
 		data2[0] = LED_REG_PSC1;
 		// clear past select data and apply PWM1
 		data1[1] = (LEDSelect & ~LEDs) | (LED_BLINK_PWM1 & LEDs);
 	}
 	else
 		return;
 	
     uint8_t error = TWI_write_data(LEDAddress, data1, 2);
     _delay_us(1);
     error = TWI_write_data(LEDAddress, data2, 2);
     _delay_us(1);
     
 	LEDSelect = data1[1];	
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
 }
     
 void clear(){
	 
	 // clear LEDs 
    setBrightness(3, 0, LED_RED | LED_GREEN | LED_BLUE);
 }
 
void errorSequence(){
	
	clear();
     
    // set blinking red lights
    setBrightness(1, 200, LED_RED);
    setBlinkRate(1, 130, LED_RED);    
}

void setDefaultColor(){
	
	clear();
		 
	 // set frequency to slowest and duty cyle to zero (off)
	 uint8_t LEDColor = eeprom::getEeprom8(eeprom_offsets::LED_STRIP_SETTINGS, 1);
	 uint32_t CustomColor = eeprom::getEeprom32(eeprom_offsets::LED_STRIP_SETTINGS + blink_eeprom_offsets::CUSTOM_COLOR_OFFSET, 0xFFFFFFFF);
	
	// blink rate has to be set first in order for color to register,
	// so set blink before each color
	 
	 switch(LEDColor){
		 case LED_DEFAULT_WHITE:
			setBlinkRate(1, blinkRate, LED_RED | LED_GREEN | LED_BLUE);
			setBrightness(1, 100, LED_RED | LED_GREEN | LED_BLUE);	
			break;
		 case LED_DEFAULT_BLUE:
			setBlinkRate(1, blinkRate, LED_BLUE);
			setBrightness(1, 100, LED_BLUE);
			break;
		 case LED_DEFAULT_RED: 
		    setBlinkRate(1, blinkRate, LED_RED);
			setBrightness(1, 100, LED_RED);
			break;
		 case LED_DEFAULT_GREEN: 
			setBlinkRate(1, blinkRate, LED_GREEN);
			setBrightness(1, 100, LED_GREEN);
			break;
		 case LED_DEFAULT_ORANGE:		
			setBlinkRate(1, blinkRate, LED_GREEN);
			setBrightness(1, 50, LED_GREEN);		
			setBlinkRate(0, blinkRate, LED_RED);
			setBrightness(0, 200, LED_RED);
			break;
		 case LED_DEFAULT_PINK:
			setBlinkRate(1, blinkRate, LED_BLUE| LED_RED);
			setBrightness(1, 70, LED_BLUE| LED_RED);
			break;
		 case LED_DEFAULT_PURPLE:
			setBlinkRate(1, blinkRate, LED_BLUE | LED_RED);
			setBrightness(1, 200, LED_BLUE | LED_RED);
			break;
		 case LED_DEFAULT_CUSTOM:
			setColor(CustomColor >> 24, CustomColor >> 16, CustomColor >> 8, true);
			break;
		 case LED_DEFAULT_OFF:
			break;
	 }
}

void setLEDBlink(uint8_t rate){
		
	blinkRate = rate;
	setDefaultColor();
}

    // set LED color and store to EEPROM "custom" color area
void setCustomColor(uint8_t red, uint8_t green, uint8_t blue){
	eeprom::setCustomColor(red, green, blue);
	setColor(red, green, blue, true);
}

#define abs(X) ((X) < 0 ? -(X) : (X)) 

// wiggly: set a three value color using a 2 value driver (+ ON/OFF channel)
void setColor(uint8_t red, uint8_t green, uint8_t blue, bool clearOld){

	if(clearOld){
		clear();}
	
	int on, count;
	on = count = 0;
    uint8_t leds_on;
	
    // if any color is all on, set it to ON
    if (red == 255)
        leds_on |= LED_RED;
    if (green == 255)
        leds_on |= LED_GREEN;
    if(blue == 255)
        leds_on |= LED_BLUE;
    
    setBrightness(3, 1, leds_on);
    
    // find number of distict color values
	if(!((red == 0) || (red == 255)))
	{	count++;}
		
	if(!((green == 0) || (green == 255)))
	{	count++;}
	
	if(!((blue == 0) || (blue == 255)))
	{	count++;}
    
	
	// we have two channels for brightness, if we have two settings
	// or less, just set the channels to the requested values
	int channel = 0;
	if(count < 3){
		if((red > 0) && (red < 255)){
            setBlinkRate(channel, blinkRate, LED_RED);
			setBrightness(channel++, red, LED_RED);
        }
		if((green > 0) && (green < 255))
        {
            setBlinkRate(channel, blinkRate, LED_GREEN);
			setBrightness(channel++, green, LED_GREEN);
        }
		if((blue > 0) && (blue < 255)){
            setBlinkRate(channel, blinkRate, LED_BLUE);
			setBrightness(channel++, blue, LED_BLUE);
        }
	}
	// if three different values are requested, set the two closest
	// values to be equal and use the same channel 
	else {
		int distRB = abs(red - blue);
		int distRG = abs(red - green);
		int distBG = abs(blue - green);
		
		if(distRB < distRG){
			/// red and blue closest
			if(distRB < distBG){
                setBlinkRate(0, blinkRate, LED_GREEN);
				setBrightness(0, green, LED_GREEN);
				setBlinkRate(1, blinkRate, LED_RED | LED_BLUE);
				setBrightness(1, red, LED_RED | LED_BLUE);
			}
			/// blue and green closest
			else{
                setBlinkRate(0, blinkRate, LED_RED);
				setBrightness(0, red, LED_RED);
				setBlinkRate(1, blinkRate, LED_GREEN |LED_BLUE);
				setBrightness(1, green, LED_GREEN | LED_BLUE);
			}
		}
		else{
			/// red and green closest
			if(distRG < distBG){
                setBlinkRate(0, blinkRate, LED_BLUE);
				setBrightness(0, blue, LED_BLUE);
				setBlinkRate(1, blinkRate, LED_GREEN | LED_RED);
				setBrightness(1, green, LED_GREEN | LED_RED);
			}
			/// blue and green closest
			else{
                setBlinkRate(0, blinkRate, LED_RED);
				setBrightness(0, red, LED_RED);
				setBlinkRate(1, blinkRate, LED_GREEN |LED_BLUE);
				setBrightness(1, green, LED_GREEN | LED_BLUE);
			}
		}
	}	
 
}
    
}
