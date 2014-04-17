#ifndef STANDARD_LIQUID_CRYSTAL_HH
#define STANDARD_LIQUID_CRYSTAL_HH

// TODO: Proper attribution

#include <stdint.h>
#include <avr/pgmspace.h>
#include "Pin.hh"
#include "LiquidCrystalSerial.hh"

// Flags for Backlight Control.
#define LCD_BACKLIGHT_ACTIVE_HIGH
//#define LCD_BACKLIGHT_ACTIVE_LOW

//I2C Adress 0x27
#define LCD_I2C_DEVICE_ADDRESS 	0x27

//Pin mapings for the I2C Bus Extender
//(Mapping pins on the LCD to the pins on the bus extender)
#define LCD_BACKLIGHT_PIN 		3
#define LCD_EN_PIN				2
#define LCD_RW_PIN				1
#define LCD_RS_PIN				0
#define LCD_D4_PIN				4
#define LCD_D5_PIN				5
#define LCD_D6_PIN				6
#define LCD_D7_PIN				7

class LiquidCrystalSerial_I2C : public LiquidCrystalSerial {

public:
  LiquidCrystalSerial_I2C();

  void init();
    
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  bool setBacklight( bool value );
  bool hasI2CDisplay();
  
private:
  void send(uint8_t, bool);
  void writeSerial(uint8_t);
  void write4bits(uint8_t value, bool dataMode);
  void pulseEnable(uint8_t value);

  bool has_i2c_lcd;
  bool backlight_state;  
};

#endif // STANDARD_LIQUID_CRYSTAL_HH
