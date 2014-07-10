#ifndef __LOCALE_HH_INCLUDED__
#define __LOCALE_HH_INCLUDED__

#include "Configuration.hh"
#include <avr/pgmspace.h>

// prog_uchar is defined:
//typedef unsigned char unsigned char;



extern const unsigned char ON_MSG[];
extern const unsigned char OFF_MSG[];

#ifdef STACK_PAINT
extern const unsigned char SPLASH_SRAM_MSG[];
#endif

//extern const unsigned char SPLASH1A_MSG[];
//extern const unsigned char SPLASH2A_MSG[];
//extern const unsigned char SPLASH3A_MSG[];
//extern const unsigned char SPLASH4A_MSG[];

extern const unsigned char GO_MSG[];
extern const unsigned char STOP_MSG[];
extern const unsigned char PLATFORM_MSG[];
extern const unsigned char TOOL_MSG[];

#if defined(NOZZLE_CALIBRATION_SCRIPT)
extern const unsigned char START_TEST_MSG[];
extern const unsigned char EXPLAIN1_MSG[];
extern const unsigned char EXPLAIN2_MSG[];
extern const unsigned char END_MSG  [];
#endif

extern const unsigned char SELECT_MSG[];
extern const unsigned char DONE_MSG[];
extern const unsigned char NO_MSG[];
extern const unsigned char YES_MSG[];
extern const uint8_t YES_NO_WIDTH;

extern const unsigned char XAXIS_MSG[];
extern const unsigned char YAXIS_MSG[];

extern const unsigned char HEATER_ERROR_MSG[];

extern const unsigned char STOP_EXIT_MSG[];
extern const unsigned char TIMEOUT_MSG[];

extern const unsigned char LOAD_RIGHT_MSG[];
extern const unsigned char LOAD_LEFT_MSG[];
extern const unsigned char LOAD_SINGLE_MSG[];
extern const unsigned char UNLOAD_SINGLE_MSG[];
extern const unsigned char UNLOAD_RIGHT_MSG[];
extern const unsigned char UNLOAD_LEFT_MSG[];

extern const unsigned char JOG1_MSG[];
extern const unsigned char JOG2X_MSG[];
extern const unsigned char JOG3X_MSG[];
extern const unsigned char JOG4X_MSG[];
extern const unsigned char JOG3Y_MSG[];
extern const unsigned char JOG3Z_MSG[];


//extern const unsigned char DISTANCESHORT_MSG[];
//extern const unsigned char DISTANCELONG_MSG[];

//#define HEATING_MSG_DEF "Heating:"
//#define HEATING_MSG_LEN (sizeof(HEATING_MSG_DEF) - 1)
#define HEATING_MSG_LEN (strlen_P((const char *)HEATING_MSG))
//#define HEATING_MSG_LEN (HeatingMsgLen)
//extern const unsigned char HeatingMsgLen;
extern const unsigned char HEATING_MSG[];
// HEATING_SPACES_MSG is a full LCD screen wide (20 spaces)
extern const unsigned char HEATING_SPACES_MSG[];


extern const unsigned char BUILD_PERCENT_MSG[];
extern const unsigned char EXTRUDER1_TEMP_MSG[];
extern const unsigned char EXTRUDER2_TEMP_MSG[];
extern const unsigned char PLATFORM_TEMP_MSG[];
extern const unsigned char EXTRUDER_TEMP_MSG[];


extern const unsigned char EXTRUDER_SPACES_MSG[];
extern const unsigned char RIGHT_SPACES_MSG[];
extern const unsigned char LEFT_SPACES_MSG[];
extern const unsigned char PLATFORM_SPACES_MSG[];
extern const unsigned char RESET1_MSG[];
extern const unsigned char RESET2_MSG[];
extern const unsigned char CANCEL_MSG[];
extern const unsigned char CANCEL_FIL_MSG[];

extern const unsigned char PAUSE_MSG[];
extern const unsigned char UNPAUSE_MSG[];
extern const unsigned char COLD_PAUSE_MSG[];

extern const unsigned char BUILDING_MSG[];
extern const unsigned char CARDCOMMS_MSG[];
extern const unsigned char CARDOPENERR_MSG[];
extern const unsigned char CARDNOFILES_MSG[];
extern const unsigned char NOCARD_MSG[];
extern const unsigned char CARDERROR_MSG[];
extern const unsigned char CARDCRC_MSG[];
extern const unsigned char CARDFORMAT_MSG[];
extern const unsigned char CARDSIZE_MSG[];
extern const unsigned char EXTRUDER_TIMEOUT_MSG[];
extern const unsigned char PLATFORM_TIMEOUT_MSG[];

extern const unsigned char BUILD_MSG[];
extern const unsigned char PREHEAT_MSG[];
extern const unsigned char UTILITIES_MSG[];
extern const unsigned char MONITOR_MSG[];
extern const unsigned char JOG_MSG[];
//extern const unsigned char CALIBRATION_MSG[];
extern const unsigned char HOME_AXES_MSG[];
extern const unsigned char FILAMENT_OPTIONS_MSG[];
extern const unsigned char VERSION_MSG[];
extern const unsigned char DSTEPS_MSG[];
extern const unsigned char ESTEPS_MSG[];
extern const unsigned char PLATE_LEVEL_MSG[];
extern const unsigned char PREHEAT_SET_MSG[];
extern const unsigned char SETTINGS_MSG[];
extern const unsigned char RESET_MSG[];
extern const unsigned char NOZZLES_MSG[];
extern const unsigned char BOT_STATS_MSG[];
extern const unsigned char TOOL_COUNT_MSG[];
extern const unsigned char SOUND_MSG[];
//extern const unsigned char LED_MSG[];
//extern const unsigned char LED_HEAT_MSG[];
extern const unsigned char EXIT_MSG[];
extern const unsigned char ACCELERATE_MSG[];
extern const unsigned char HBP_MSG[];
extern const unsigned char OVERRIDE_GCODE_TEMP_MSG[];
extern const unsigned char PAUSE_HEAT_MSG[];
extern const unsigned char EXTRUDER_HOLD_MSG[];
extern const unsigned char SD_USE_CRC_MSG[];
extern const unsigned char PSTOP_ENABLE_MSG[];
extern const unsigned char DISABLED_MSG[];
#ifdef DITTO_PRINT
extern const unsigned char DITTO_PRINT_MSG[];
#endif
extern const unsigned char PAUSEATZPOS_MSG[];
extern const unsigned char CHANGE_SPEED_MSG[];
extern const unsigned char CHANGE_TEMP_MSG[];
extern const unsigned char FAN_ON_MSG[];
extern const unsigned char FAN_OFF_MSG[];
extern const unsigned char RETURN_TO_MAIN_MSG[];
extern const unsigned char PRINT_ANOTHER_MSG[];
extern const unsigned char CANNOT_PRINT_ANOTHER_MSG[];


//extern const unsigned char RED_COLOR_MSG[];
//extern const unsigned char ORANGE_COLOR_MSG[];
//extern const unsigned char PINK_COLOR_MSG[];
//extern const unsigned char GREEN_COLOR_MSG[];
//extern const unsigned char BLUE_COLOR_MSG[];
//extern const unsigned char PURPLE_COLOR_MSG[];
//extern const unsigned char WHITE_COLOR_MSG[];
//extern const unsigned char CUSTOM_COLOR_MSG[];
//extern const unsigned char OFF_COLOR_MSG[];

extern const unsigned char ERROR_MSG[];
extern const unsigned char NA_MSG[];
extern const unsigned char WAITING_MSG[];

extern const unsigned char EXTEMP_CHANGE_MSG[];
extern const unsigned char HEATER_INACTIVITY_MSG[];
extern const unsigned char HEATER_TOOL_MSG[];
extern const unsigned char HEATER_TOOL0_MSG[];
extern const unsigned char HEATER_TOOL1_MSG[];
extern const unsigned char HEATER_PLATFORM_MSG[];
//
extern const unsigned char HEATER_FAIL_SOFTWARE_CUTOFF_MSG[];
extern const unsigned char HEATER_FAIL_NOT_HEATING_MSG[];
extern const unsigned char HEATER_FAIL_DROPPING_TEMP_MSG[];
extern const unsigned char HEATER_FAIL_NOT_PLUGGED_IN_MSG[];
extern const unsigned char HEATER_FAIL_READ_MSG[];
//
extern const unsigned char BUILD_TIME_MSG[];
extern const unsigned char Z_POSITION_MSG[];
extern const unsigned char FILAMENT_MSG[];
extern const unsigned char MON_FILAMENT_MSG[];
extern const unsigned char PROFILES_MSG[];
extern const unsigned char HOME_OFFSETS_MSG[];
extern const unsigned char MILLIMETERS_MSG[];
extern const unsigned char METERS_MSG[];
extern const unsigned char LINE_NUMBER_MSG[];
//extern const unsigned char LEFT_EXIT_MSG [];

extern const unsigned char BACK_TO_MONITOR_MSG[];
extern const unsigned char STATS_MSG[];
extern const unsigned char CANCEL_BUILD_MSG[];
extern const unsigned char HEATERS_OFF_MSG[];

extern const unsigned char FILAMENT_ODOMETER_MSG[];
extern const unsigned char FILAMENT_LIFETIME1_MSG[];
extern const unsigned char FILAMENT_TRIP1_MSG[];
extern const unsigned char FILAMENT_LIFETIME2_MSG[];
extern const unsigned char FILAMENT_TRIP2_MSG[];
extern const unsigned char FILAMENT_RESET_TRIP_MSG[];

extern const unsigned char PROFILE_RESTORE_MSG[];
extern const unsigned char PROFILE_DISPLAY_CONFIG_MSG[];
extern const unsigned char PROFILE_CHANGE_NAME_MSG[];
extern const unsigned char PROFILE_SAVE_TO_PROFILE_MSG[];
extern const unsigned char PROFILE_PROFILE_NAME_MSG[];
extern const unsigned char UPDNLRM_MSG[];
extern const unsigned char UPDNLM_MSG[];
extern const unsigned char XYZOFFSET_MSG[];
extern const unsigned char PROFILE_RIGHT_MSG[];
extern const unsigned char PROFILE_LEFT_MSG[];
extern const unsigned char PROFILE_PLATFORM_MSG[];
extern const unsigned char PAUSE_AT_ZPOS_MSG[];

extern const unsigned char PRINTED_TOO_LONG_MSG[];

extern const unsigned char PAUSE_ENTER_MSG[];
extern const unsigned char CANCELLING_ENTER_MSG[];
extern const unsigned char PAUSE_LEAVE_MSG[];
extern const unsigned char PAUSE_DRAINING_PIPELINE_MSG[];
extern const unsigned char PAUSE_CLEARING_BUILD_MSG[];
extern const unsigned char PAUSE_RESUMING_POSITION_MSG[];
extern const unsigned char TOTAL_TIME_MSG[];
extern const unsigned char LAST_TIME_MSG[];
extern const unsigned char BUILD_TIME2_MSG[];
#ifdef EEPROM_MENU_ENABLE
extern const unsigned char EEPROM_MSG[];
extern const unsigned char EEPROM_DUMP_MSG[];
extern const unsigned char EEPROM_RESTORE_MSG[];
extern const unsigned char EEPROM_ERASE_MSG[];
#endif

extern const unsigned char ERROR_STREAM_VERSION[];

extern const unsigned char ERROR_BOT_TYPE[];

#ifdef ALTERNATE_UART
extern const unsigned char ALT_UART_MSG[];
extern const unsigned char ALT_UART_0_MSG[];
extern const unsigned char ALT_UART_1_MSG[];
#endif

#ifdef AUTO_LEVEL
extern const unsigned char ALEVEL_SCREEN_MSG1[];
extern const unsigned char ALEVEL_SCREEN_MSG2[];
extern const unsigned char ALEVEL_UTILITY_MSG[];
extern const unsigned char ALEVEL_BADLEVEL_MSG[];
extern const unsigned char ALEVEL_COLINEAR_MSG[];
extern const unsigned char ALEVEL_INCOMPLETE_MSG[];
extern const unsigned char ALEVEL_INACTIVE_MSG[];
extern const unsigned char ALEVEL_ACTIVE_MSG[];
#endif

extern const unsigned char MON_ZPOS_MSG[];

#if defined(ESTIMATE_TIME)
extern const unsigned char MON_TIME_LEFT_MSG[];
extern const unsigned char MON_TIME_LEFT_SECS_MSG[];
#endif

#if defined(BUILD_STATS)
extern const unsigned char MON_ELAPSED_TIME_MSG[];
#endif

#if defined(MACHINE_ID_MENU)
extern const unsigned char MACHINE_ID_MSG[];
extern const unsigned char MACHINE_ID_REP1_MSG[];
extern const unsigned char MACHINE_ID_REP2_MSG[];
extern const unsigned char MACHINE_ID_REP2X_MSG[];
#endif

#endif // __LOCALE_HH_INCLUDED__
