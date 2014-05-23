#ifndef __LOCALE_HH_INCLUDED__
#define __LOCALE_HH_INCLUDED__

#include "Configuration.hh"
#include <avr/pgmspace.h>

// prog_uchar is defined:
//typedef unsigned char PROGMEM prog_uchar;



extern const PROGMEM prog_uchar ON_MSG[];
extern const PROGMEM prog_uchar OFF_MSG[];

#ifdef STACK_PAINT
extern const PROGMEM prog_uchar SPLASH_SRAM_MSG[];
#endif

//extern const PROGMEM prog_uchar SPLASH1A_MSG[];
//extern const PROGMEM prog_uchar SPLASH2A_MSG[];
//extern const PROGMEM prog_uchar SPLASH3A_MSG[];
//extern const PROGMEM prog_uchar SPLASH4A_MSG[];

extern const PROGMEM prog_uchar GO_MSG[];
extern const PROGMEM prog_uchar STOP_MSG[];
extern const PROGMEM prog_uchar PLATFORM_MSG[];
extern const PROGMEM prog_uchar TOOL_MSG[];
//extern const PROGMEM prog_uchar START_TEST_MSG[];
//extern const PROGMEM prog_uchar EXPLAIN1_MSG[];
//extern const PROGMEM prog_uchar EXPLAIN2_MSG[];
//extern const PROGMEM prog_uchar END_MSG  [];

extern const PROGMEM prog_uchar SELECT_MSG[];
extern const PROGMEM prog_uchar DONE_MSG[];
extern const PROGMEM prog_uchar NO_MSG[];
extern const PROGMEM prog_uchar YES_MSG[];

extern const PROGMEM prog_uchar XAXIS_MSG[];
extern const PROGMEM prog_uchar YAXIS_MSG[];

extern const PROGMEM prog_uchar HEATER_ERROR_MSG[];

extern const PROGMEM prog_uchar STOP_EXIT_MSG[];
extern const PROGMEM prog_uchar TIMEOUT_MSG[];

extern const PROGMEM prog_uchar LOAD_RIGHT_MSG[];
extern const PROGMEM prog_uchar LOAD_LEFT_MSG[];
extern const PROGMEM prog_uchar LOAD_SINGLE_MSG[];
extern const PROGMEM prog_uchar UNLOAD_SINGLE_MSG[];
extern const PROGMEM prog_uchar UNLOAD_RIGHT_MSG[];
extern const PROGMEM prog_uchar UNLOAD_LEFT_MSG[];

extern const PROGMEM prog_uchar JOG1_MSG[];
extern const PROGMEM prog_uchar JOG2X_MSG[];
extern const PROGMEM prog_uchar JOG3X_MSG[];
extern const PROGMEM prog_uchar JOG4X_MSG[];
extern const PROGMEM prog_uchar JOG3Y_MSG[];
extern const PROGMEM prog_uchar JOG3Z_MSG[];


//extern const PROGMEM prog_uchar DISTANCESHORT_MSG[];
//extern const PROGMEM prog_uchar DISTANCELONG_MSG[];

//#define HEATING_MSG_DEF "Heating:"
//#define HEATING_MSG_LEN (sizeof(HEATING_MSG_DEF) - 1)
#define HEATING_MSG_LEN (strlen_P((const char *)HEATING_MSG))
//#define HEATING_MSG_LEN (HeatingMsgLen)
//extern const uint8_t PROGMEM HeatingMsgLen;
extern const PROGMEM prog_uchar HEATING_MSG[];
// HEATING_SPACES_MSG is a full LCD screen wide (20 spaces)
extern const PROGMEM prog_uchar HEATING_SPACES_MSG[];


extern const PROGMEM prog_uchar BUILD_PERCENT_MSG[];
extern const PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[];
extern const PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[];
extern const PROGMEM prog_uchar PLATFORM_TEMP_MSG[];
extern const PROGMEM prog_uchar EXTRUDER_TEMP_MSG[];


extern const PROGMEM prog_uchar EXTRUDER_SPACES_MSG[];
extern const PROGMEM prog_uchar RIGHT_SPACES_MSG[];
extern const PROGMEM prog_uchar LEFT_SPACES_MSG[];
extern const PROGMEM prog_uchar PLATFORM_SPACES_MSG[];
extern const PROGMEM prog_uchar RESET1_MSG[];
extern const PROGMEM prog_uchar RESET2_MSG[];
extern const PROGMEM prog_uchar CANCEL_MSG[];
extern const PROGMEM prog_uchar CANCEL_FIL_MSG[];

extern const PROGMEM prog_uchar PAUSE_MSG[];
extern const PROGMEM prog_uchar UNPAUSE_MSG[];
extern const PROGMEM prog_uchar COLD_PAUSE_MSG[];

extern const PROGMEM prog_uchar BUILDING_MSG[];
extern const PROGMEM prog_uchar CARDCOMMS_MSG[];
extern const PROGMEM prog_uchar CARDOPENERR_MSG[];
extern const PROGMEM prog_uchar CARDNOFILES_MSG[];
extern const PROGMEM prog_uchar NOCARD_MSG[];
extern const PROGMEM prog_uchar CARDERROR_MSG[];
extern const PROGMEM prog_uchar CARDCRC_MSG[];
extern const PROGMEM prog_uchar CARDFORMAT_MSG[];
extern const PROGMEM prog_uchar CARDSIZE_MSG[];
extern const PROGMEM prog_uchar EXTRUDER_TIMEOUT_MSG[];
extern const PROGMEM prog_uchar PLATFORM_TIMEOUT_MSG[];

extern const PROGMEM prog_uchar BUILD_MSG[];
extern const PROGMEM prog_uchar PREHEAT_MSG[];
extern const PROGMEM prog_uchar UTILITIES_MSG[];
extern const PROGMEM prog_uchar MONITOR_MSG[];
extern const PROGMEM prog_uchar JOG_MSG[];
//extern const PROGMEM prog_uchar CALIBRATION_MSG[];
extern const PROGMEM prog_uchar HOME_AXES_MSG[];
extern const PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[];
extern const PROGMEM prog_uchar VERSION_MSG[];
extern const PROGMEM prog_uchar DSTEPS_MSG[];
extern const PROGMEM prog_uchar ESTEPS_MSG[];
extern const PROGMEM prog_uchar PLATE_LEVEL_MSG[];
extern const PROGMEM prog_uchar PREHEAT_SET_MSG[];
extern const PROGMEM prog_uchar SETTINGS_MSG[];
extern const PROGMEM prog_uchar RESET_MSG[];
extern const PROGMEM prog_uchar NOZZLES_MSG[];
extern const PROGMEM prog_uchar BOT_STATS_MSG[];
extern const PROGMEM prog_uchar TOOL_COUNT_MSG[];
extern const PROGMEM prog_uchar SOUND_MSG[];
//extern const PROGMEM prog_uchar LED_MSG[];
//extern const PROGMEM prog_uchar LED_HEAT_MSG[];
extern const PROGMEM prog_uchar EXIT_MSG[];
extern const PROGMEM prog_uchar ACCELERATE_MSG[];
extern const PROGMEM prog_uchar HBP_MSG[];
extern const PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[];
extern const PROGMEM prog_uchar PAUSE_HEAT_MSG[];
extern const PROGMEM prog_uchar EXTRUDER_HOLD_MSG[];
extern const PROGMEM prog_uchar SD_USE_CRC_MSG[];
extern const PROGMEM prog_uchar PSTOP_ENABLE_MSG[];
extern const PROGMEM prog_uchar DISABLED_MSG[];
#ifdef DITTO_PRINT
extern const PROGMEM prog_uchar DITTO_PRINT_MSG[];
#endif
extern const PROGMEM prog_uchar PAUSEATZPOS_MSG[];
extern const PROGMEM prog_uchar CHANGE_SPEED_MSG[];
extern const PROGMEM prog_uchar CHANGE_TEMP_MSG[];
extern const PROGMEM prog_uchar FAN_ON_MSG[];
extern const PROGMEM prog_uchar FAN_OFF_MSG[];
extern const PROGMEM prog_uchar RETURN_TO_MAIN_MSG[];
extern const PROGMEM prog_uchar PRINT_ANOTHER_MSG[];
extern const PROGMEM prog_uchar CANNOT_PRINT_ANOTHER_MSG[];


//extern const PROGMEM prog_uchar RED_COLOR_MSG[];
//extern const PROGMEM prog_uchar ORANGE_COLOR_MSG[];
//extern const PROGMEM prog_uchar PINK_COLOR_MSG[];
//extern const PROGMEM prog_uchar GREEN_COLOR_MSG[];
//extern const PROGMEM prog_uchar BLUE_COLOR_MSG[];
//extern const PROGMEM prog_uchar PURPLE_COLOR_MSG[];
//extern const PROGMEM prog_uchar WHITE_COLOR_MSG[];
//extern const PROGMEM prog_uchar CUSTOM_COLOR_MSG[];
//extern const PROGMEM prog_uchar OFF_COLOR_MSG[];

extern const PROGMEM prog_uchar ERROR_MSG[];
extern const PROGMEM prog_uchar NA_MSG[];
extern const PROGMEM prog_uchar WAITING_MSG[];

extern const PROGMEM prog_uchar EXTEMP_CHANGE_MSG[];
extern const PROGMEM prog_uchar HEATER_INACTIVITY_MSG[];
extern const PROGMEM prog_uchar HEATER_TOOL_MSG[];
extern const PROGMEM prog_uchar HEATER_TOOL0_MSG[];
extern const PROGMEM prog_uchar HEATER_TOOL1_MSG[];
extern const PROGMEM prog_uchar HEATER_PLATFORM_MSG[];
//
extern const PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[];
extern const PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[];
extern const PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[];
extern const PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[];
extern const PROGMEM prog_uchar HEATER_FAIL_READ_MSG[];
//
extern const PROGMEM prog_uchar BUILD_TIME_MSG[];
extern const PROGMEM prog_uchar Z_POSITION_MSG[];
extern const PROGMEM prog_uchar FILAMENT_MSG[];
extern const PROGMEM prog_uchar PROFILES_MSG[];
extern const PROGMEM prog_uchar HOME_OFFSETS_MSG[];
extern const PROGMEM prog_uchar MILLIMETERS_MSG[];
extern const PROGMEM prog_uchar METERS_MSG[];
extern const PROGMEM prog_uchar LINE_NUMBER_MSG[];
//extern const PROGMEM prog_uchar LEFT_EXIT_MSG [];

extern const PROGMEM prog_uchar BACK_TO_MONITOR_MSG[];
extern const PROGMEM prog_uchar STATS_MSG[];
extern const PROGMEM prog_uchar CANCEL_BUILD_MSG[];
extern const PROGMEM prog_uchar HEATERS_OFF_MSG[];

extern const PROGMEM prog_uchar FILAMENT_ODOMETER_MSG[];
extern const PROGMEM prog_uchar FILAMENT_LIFETIME1_MSG[];
extern const PROGMEM prog_uchar FILAMENT_TRIP1_MSG[];
extern const PROGMEM prog_uchar FILAMENT_LIFETIME2_MSG[];
extern const PROGMEM prog_uchar FILAMENT_TRIP2_MSG[];
extern const PROGMEM prog_uchar FILAMENT_RESET_TRIP_MSG[];

extern const PROGMEM prog_uchar PROFILE_RESTORE_MSG[];
extern const PROGMEM prog_uchar PROFILE_DISPLAY_CONFIG_MSG[];
extern const PROGMEM prog_uchar PROFILE_CHANGE_NAME_MSG[];
extern const PROGMEM prog_uchar PROFILE_SAVE_TO_PROFILE_MSG[];
extern const PROGMEM prog_uchar PROFILE_PROFILE_NAME_MSG[];
extern const PROGMEM prog_uchar UPDNLRM_MSG[];
extern const PROGMEM prog_uchar UPDNLM_MSG[];
extern const PROGMEM prog_uchar XYZOFFSET_MSG[];
extern const PROGMEM prog_uchar PROFILE_RIGHT_MSG[];
extern const PROGMEM prog_uchar PROFILE_LEFT_MSG[];
extern const PROGMEM prog_uchar PROFILE_PLATFORM_MSG[];
extern const PROGMEM prog_uchar PAUSE_AT_ZPOS_MSG[];

extern const PROGMEM prog_uchar PRINTED_TOO_LONG_MSG[];

extern const PROGMEM prog_uchar PAUSE_ENTER_MSG[];
extern PROGMEM const prog_uchar CANCELLING_ENTER_MSG[];
extern const PROGMEM prog_uchar PAUSE_LEAVE_MSG[];
extern const PROGMEM prog_uchar PAUSE_DRAINING_PIPELINE_MSG[];
extern const PROGMEM prog_uchar PAUSE_CLEARING_BUILD_MSG[];
extern const PROGMEM prog_uchar PAUSE_RESUMING_POSITION_MSG[];
extern const PROGMEM prog_uchar TOTAL_TIME_MSG[];
extern const PROGMEM prog_uchar LAST_TIME_MSG[];
extern const PROGMEM prog_uchar BUILD_TIME2_MSG[];
#ifdef EEPROM_MENU_ENABLE
extern const PROGMEM prog_uchar EEPROM_MSG[];
extern const PROGMEM prog_uchar EEPROM_DUMP_MSG[];
extern const PROGMEM prog_uchar EEPROM_RESTORE_MSG[];
extern const PROGMEM prog_uchar EEPROM_ERASE_MSG[];
#endif

extern const PROGMEM prog_uchar ERROR_STREAM_VERSION[];

extern const PROGMEM prog_uchar ERROR_BOT_TYPE[];

#endif // __LOCALE_HH_INCLUDED__

