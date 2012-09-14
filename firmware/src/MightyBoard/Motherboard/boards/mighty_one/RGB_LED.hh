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
 
 #ifndef RGB_LED_HH
 #define RGB_LED_HH
 
 // LED control registers
 #define LED_REG_PSC0	0b00000001
 #define LED_REG_PWM0	0b00000010
 #define LED_REG_PSC1	0b00000011
 #define LED_REG_PWM1	0b00000100
 #define LED_REG_SELECT  0b00000101
 
 
 // LED output types
 #define LED_BLINK_PWM0 0b10101010
 #define LED_BLINK_PWM1 0b11111111
 #define LED_ON 	0b01010101
 #define LED_OFF	0b00000000
 
 // RBG IDs
 #define LED_GREEN 0b00001100
 #define LED_RED 0b00000011
 #define LED_BLUE 0b00110000
 
 // Channel IDs
 #define LED_CHANNEL1	0
 #define LED_CHANNEL2	1	

enum LEDColors{
    LED_DEFAULT_WHITE,
    LED_DEFAULT_RED,
    LED_DEFAULT_ORANGE,
    LED_DEFAULT_PINK,
    LED_DEFAULT_GREEN,
    LED_DEFAULT_BLUE,
    LED_DEFAULT_PURPLE,
    LED_DEFAULT_OFF,
    LED_DEFAULT_CUSTOM
};
 
 
 #include "Types.hh"
 
 namespace RGB_LED {

 void init(); 
 void startupSequence();
 void errorSequence();
 void setColor(uint8_t red, uint8_t green, uint8_t blue, bool clearOld);
 void setLEDBlink(uint8_t rate);
 void setDefaultColor();
 void setCustomColor(uint8_t red, uint8_t green, uint8_t blue);
 
 void clear();
 
 void setBrightness(uint8_t level, uint8_t LED1, uint8_t LED2);
 void setBlinkRate(uint8_t rate, uint8_t LED1, uint8_t LED2);
 void toggleLEDNoPWM(bool enable, uint8_t LED);
 
 }
 
 #endif
 
 
