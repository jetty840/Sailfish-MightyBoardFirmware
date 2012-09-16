/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
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

#ifndef HEATER_H
#define HEATER_H

#include "TemperatureSensor.hh"
#include "HeatingElement.hh"
#include "Pin.hh"
#include "PID.hh"
#include "Types.hh"
#include "Timeout.hh"

#define DEFAULT_P 7.0
#define DEFAULT_I 0.325
#define DEFAULT_D 36.0

enum HeaterFailMode{
	HEATER_FAIL_NONE = 0,
	HEATER_FAIL_NOT_PLUGGED_IN = 0x02,
	HEATER_FAIL_SOFTWARE_CUTOFF = 0x04,
	HEATER_FAIL_NOT_HEATING = 0x08,
	HEATER_FAIL_DROPPING_TEMP = 0x10,
	HEATER_FAIL_BAD_READS = 0x20
};



/// A heater object uses a #TemperatureSensor to control a #HeatingElement.
/// It is primarily a #PID controller, but enters a bypass mode when the setpoint
/// temperature is much higher than the current temperature. The #manage_temperature()
/// function must be called periodically for it to be able to maintain accurate
/// temperature control.
/// \ingroup SoftwareLibraries
class Heater
{
  private:
    TemperatureSensor& sensor;          ///< Sensor used to measure input temperature
    HeatingElement& element;            ///< Heating element used to produce an output
    Timeout next_pid_timeout;           ///< Timeout timer for PID loop (should be slower
                                        ///< or the same speed as sensor timeout)
    Timeout next_sense_timeout;         ///< Timeout timer for sensor measurement
    micros_t sample_interval_micros;    ///< Interval that the temperature sensor should
                                        ///< be updated at.

    volatile uint16_t current_temperature;       ///< Last known temperature reading
    uint16_t startTemp;					///< start temperature when new target is set.  used to assess heating up progress 
	uint16_t paused_set_temperature;		///< we record the set temperature when a heater is "paused"
    bool newTargetReached;				///< flag set when heater reached target and cleared when a new temperature is set
    
    uint16_t eeprom_base;               ///< Base address to read EEPROM configuration from

    PID pid;                            ///< PID controller instance
    bool bypassing_PID;                 ///< True if the heater is in full on

    bool fail_state;                    ///< True if the heater has detected a hardware
                                        ///< failure and is shut down.
    uint8_t fail_count;                 ///< Count of the number of hardware failures that
                                        ///< have been reported by #getTemperature().
                                        ///< If this goes over #SENSOR_MAX_BAD_READINGS,
                                        ///< then the heater will go into a fail state.
    uint8_t value_fail_count;			///< a second failure counter for valid temp reads that are out of range (eg too hot)
    HeaterFailMode fail_mode;			///< queryable state to indicate WHY the heater fails

    Timeout heatingUpTimer;				///< timeout indicating how long heater has been heating
    Timeout heatProgressTimer;			///< timeout to flag if heater is not heating up from start
    bool progressChecked;				///< flag that heating up progress has been checked.
    const bool heat_timing_check;       ///< allow disabling of heat progress timing for heated build platform. 
    bool is_paused;						///< set to true when we wish to pause the heater from heating up 
    bool is_disabled;					///< heaters are disabled when they are not present (user settable)

    /// This is the interval between PID calculations.  It doesn't make sense for
    /// this to be fast (<1 sec) because of the long system delay between heater
    /// and sensor.
    const static micros_t UPDATE_INTERVAL_MICROS = 500L * 1000L;

    /// Put the heater into a failure state, ensuring that the heating element is
    /// disabled.
    void fail();

  public:
    /// Instantiate a new heater object.
    /// \param[in] sensor #TemperatureSensor element to use as an input
    /// \param[in] element #HeatingElement to use as an output
    /// \param[in] sample_interval_micros Interval to sample the temperature sensor,
    ///                                    in microseconds.
    /// \param[in] eeprom_base EEPROM address where the PID settings are stored.
    Heater(TemperatureSensor& sensor,
           HeatingElement& element,
           const micros_t sample_interval_micros,
           const uint16_t eeprom_base,
           bool heat_timing_check);
    
    /// Get the current sensor temperature
    /// \return Current sensor temperature, in degrees Celcius
    int get_current_temperature();

    /// Get the setpoint temperature
    /// \return Setpoint temperature, in degrees Celcius
    int get_set_temperature();

    /// Set the target output temperature
    /// \param temp New target temperature, in degrees Celcius.
    void set_target_temperature(int temp);

    /// Check if the heater is within the specified band
    /// \return True if the heater temperature is within #TARGET_HYSTERESIS degrees
    ///         of the setpoint temperature.
    bool has_reached_target_temperature();

    /// Check if the heater is in a failure state
    /// \return true if the heater has failed.
    bool has_failed();

    /// Run the heater management loop. This must be called periodically,
    /// at a higher frequency than #sample_interval_micros.
    void manage_temperature();

    /// Change the setpoint temperature
    /// \param value New setpoint temperature, in degrees Celcius.
    void set_output(uint8_t value);

    /// Reset the heater to a to board-on state
    void reset();
    
    // pause or unpause the heater
    // a paused heater will not heat
    void Pause(bool on);
    
    bool isPaused() { return is_paused;}

    /// Get the current PID error term
    /// \return E term from the PID controller
    int getPIDErrorTerm();

    /// Get the current PID delta term
    /// \return D term from the PID controller
    int getPIDDeltaTerm();

    /// Get the last PID output
    /// \return last output from the PID controller
    int getPIDLastOutput();
    
    /// get the difference between the current temperature
    /// and the set temperature
    int getDelta();
    
    /// is heater temperature target different than current
    bool isHeating();
    
    /// is heater temperature target less than current temperature
    bool isCooling();
    
    /// get heater fail mode
    uint8_t GetFailMode();
    
    void disable(bool on);

    bool isDisabled(){return is_disabled;}
};

#endif // HEATER_H
