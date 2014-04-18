/* StandardLiquidCrystalSerial
 *
 * This is an implementation of communciation routines for the
 * Makerbot OEM display hardware.
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
  Pin _data_pin;   // LOW: write to LCD.  HIGH: read from LCD.
  Pin _clk_pin;    // activated by a HIGH pulse.
};

#endif // STANDARD_LIQUID_CRYSTAL_HH
