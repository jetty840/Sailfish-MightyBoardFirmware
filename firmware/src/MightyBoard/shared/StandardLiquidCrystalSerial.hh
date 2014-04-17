#ifndef STANDARD_LIQUID_CRYSTAL_HH
#define STANDARD_LIQUID_CRYSTAL_HH

// TODO: Proper attribution

#include <stdint.h>
#include <avr/pgmspace.h>
#include "Pin.hh"
#include "LiquidCrystalSerial.hh"

class StandardLiquidCrystalSerial : public LiquidCrystalSerial {

public:
  StandardLiquidCrystalSerial(Pin strobe, Pin data, Pin CLK);    
  void init(Pin strobe, Pin data, Pin CLK);  
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

private:
  void send(uint8_t, bool);
  void writeSerial(uint8_t);
  void write4bits(uint8_t value, bool dataMode);
  void pulseEnable(uint8_t value);

  Pin _strobe_pin; // LOW: command.  HIGH: character.
  Pin _data_pin; // LOW: write to LCD.  HIGH: read from LCD.
  Pin _clk_pin; // activated by a HIGH pulse.
  
};

#endif // STANDARD_LIQUID_CRYSTAL_HH
