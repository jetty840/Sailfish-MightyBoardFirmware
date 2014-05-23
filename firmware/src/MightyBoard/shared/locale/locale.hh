#ifndef __LOCALE_HH_INCLUDED__
#define __LOCALE_HH_INCLUDED__

#include "Configuration.hh"
#include <avr/pgmspace.h>

// prog_uchar is defined:
//typedef unsigned char PROGMEM prog_uchar;



extern const PROGMEM prog_uchar ON_MSG[];    // =      "ON ";
extern const PROGMEM prog_uchar OFF_MSG[];   // =     "OFF";

#ifdef STACK_PAINT
extern const PROGMEM prog_uchar SPLASH_SRAM_MSG[];  // = "Free SRAM ";
#endif
//
//extern const PROGMEM prog_uchar SPLASH1A_MSG[]; // = "    FAIL!           ";
//extern const PROGMEM prog_uchar SPLASH2A_MSG[]; // = "    SUCCESS!        ";
//extern const PROGMEM prog_uchar SPLASH3A_MSG[]; // = "connected correctly ";
//extern const PROGMEM prog_uchar SPLASH4A_MSG[]; // = "Heaters are not     ";
//
extern const PROGMEM prog_uchar GO_MSG[];   // =         "Start Preheating";
extern const PROGMEM prog_uchar STOP_MSG[]; // =       "Turn Heaters Off";
extern const PROGMEM prog_uchar PLATFORM_MSG[]; //=   "Platform";
extern const PROGMEM prog_uchar TOOL_MSG[]; //=       "Extruder";
//extern const PROGMEM prog_uchar START_TEST_MSG[];   //=  "I'm going to print  " "a series of lines so" "we can find my      " "nozzle alignment.   ";
//extern const PROGMEM prog_uchar EXPLAIN1_MSG[]; //=   "Look for the best   " "matched line in each" "axis set.  Lines are" "numbered 1-13 and...";
//extern const PROGMEM prog_uchar EXPLAIN2_MSG[]; //=   "line one is extra   " "long. The Y axis set" "is left on the plate" "and X axis is right.";
//extern const PROGMEM prog_uchar END_MSG  []; //=      "Great!  I've saved  " "these settings and  " "I'll use them to    " "make nice prints!   ";

extern const PROGMEM prog_uchar SELECT_MSG[]; //=     "Select best lines.";
extern const PROGMEM prog_uchar DONE_MSG[]; //  =     "Done";
extern const PROGMEM prog_uchar NO_MSG[]; //  =       "NO ";
extern const PROGMEM prog_uchar YES_MSG[]; // =       "YES";

extern const PROGMEM prog_uchar XAXIS_MSG[]; //=      "X Axis Line";
extern const PROGMEM prog_uchar YAXIS_MSG[]; //=      "Y Axis Line";

extern const PROGMEM prog_uchar HEATER_ERROR_MSG[]; // =  "Extruders are not   " "heating.  Check the " "wiring.";

extern const PROGMEM prog_uchar STOP_EXIT_MSG[]; //  = "Begin loading or    " "unloading. Press 'M'" "to exit.";
extern const PROGMEM prog_uchar TIMEOUT_MSG[]; // 	  = "5 minute timeout has" "elapsed.  Press 'M' " "to exit.";

extern const PROGMEM prog_uchar LOAD_RIGHT_MSG[]; //   = "Load right";
extern const PROGMEM prog_uchar LOAD_LEFT_MSG[]; //    = "Load left";
extern const PROGMEM prog_uchar LOAD_SINGLE_MSG[]; //  = "Load";
extern const PROGMEM prog_uchar UNLOAD_SINGLE_MSG[]; //= "Unload";
extern const PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]; // = "Unload right";
extern const PROGMEM prog_uchar UNLOAD_LEFT_MSG[]; //  = "Unload left";

extern const PROGMEM prog_uchar JOG1_MSG[]; // = "     Jog mode       ";
extern const PROGMEM prog_uchar JOG2X_MSG[]; //= "        X+          ";
extern const PROGMEM prog_uchar JOG3X_MSG[]; //= "      (Back)   Y->  ";
extern const PROGMEM prog_uchar JOG4X_MSG[]; //= "        X-          ";
extern const PROGMEM prog_uchar JOG3Y_MSG[]; //= "  <-X (Back)  Z->   ";
extern const PROGMEM prog_uchar JOG3Z_MSG[]; //= "  <-Y (Back)        ";


//extern const PROGMEM prog_uchar DISTANCESHORT_MSG[]; //= "SHORT";
//extern const PROGMEM prog_uchar DISTANCELONG_MSG[]; //=  "LONG";

//#define HEATING_MSG_DEF "Heating:"
//#define HEATING_MSG_LEN (sizeof(HEATING_MSG_DEF) - 1)
#define HEATING_MSG_LEN (strlen_P((const char *)HEATING_MSG))
//#define HEATING_MSG_LEN (HeatingMsgLen)
//extern const uint8_t PROGMEM HeatingMsgLen; // = (sizeof(HEATING_MSG_DEF) - 1);
extern const PROGMEM prog_uchar HEATING_MSG[]; //=        HEATING_MSG_DEF;
// HEATING_SPACES_MSG is a full LCD screen wide (20 spaces)
extern const PROGMEM prog_uchar HEATING_SPACES_MSG[]; //= HEATING_MSG_DEF "            ";


extern const PROGMEM prog_uchar BUILD_PERCENT_MSG[]; //=    " --%";
extern const PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[]; //=   "R Extruder: ---/---C";
extern const PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[]; //=   "L Extruder: ---/---C";
extern const PROGMEM prog_uchar PLATFORM_TEMP_MSG[]; // =   "Platform:   ---/---C";
extern const PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]; // =   "Extruder:   ---/---C";


extern const PROGMEM prog_uchar EXTRUDER_SPACES_MSG[]; // = "Extruder           ";
extern const PROGMEM prog_uchar RIGHT_SPACES_MSG[]; //    = "Right Tool         ";
extern const PROGMEM prog_uchar LEFT_SPACES_MSG[]; //     = "Left Tool          ";
extern const PROGMEM prog_uchar PLATFORM_SPACES_MSG[]; // = "Platform           ";
extern const PROGMEM prog_uchar RESET1_MSG[]; //          = "Restore factory";
extern const PROGMEM prog_uchar RESET2_MSG[]; //          = "settings?";
extern const PROGMEM prog_uchar CANCEL_MSG[]; //= "Cancel this print?";
extern const PROGMEM prog_uchar CANCEL_FIL_MSG[]; //= "Cancel load/unload?";

extern const PROGMEM prog_uchar PAUSE_MSG[]; //     = "Pause  "; // must be same length as the next msg
extern const PROGMEM prog_uchar UNPAUSE_MSG[]; //   = "Unpause"; // must be same length as the prior msg
extern const PROGMEM prog_uchar COLD_PAUSE_MSG[]; //= "Cold Pause";

extern const PROGMEM prog_uchar BUILDING_MSG[]; //   = "I'm already building";
extern const PROGMEM prog_uchar CARDCOMMS_MSG[]; //  = "SD card reads are   " "not going well.  Try" "a different card";
extern const PROGMEM prog_uchar CARDOPENERR_MSG[]; //= "Cannot open the file";
extern const PROGMEM prog_uchar CARDNOFILES_MSG[]; //= "SD card is empty";
extern const PROGMEM prog_uchar NOCARD_MSG[]; //     = "SD card not present";
extern const PROGMEM prog_uchar CARDERROR_MSG[]; //  = "SD card read error";
extern const PROGMEM prog_uchar CARDCRC_MSG[]; //    = "SD card read error. " "Too many CRC errors." "Bad card contacts or" "electrical noise.";
extern const PROGMEM prog_uchar CARDFORMAT_MSG[]; // = "Unable to read this " "SD card format.     " "Reformat as FAT-16. ";
extern const PROGMEM prog_uchar CARDSIZE_MSG[]; //   = "Unable to read SD   " "card partitions over" "2GB in size.        " "Reformat as FAT-16.";
extern const PROGMEM prog_uchar EXTRUDER_TIMEOUT_MSG[]; //= "I timed out while   attempting to heat  my extruder.";
extern const PROGMEM prog_uchar PLATFORM_TIMEOUT_MSG[]; //= "I timed out while   attempting to heat  my platform.";
//
extern const PROGMEM prog_uchar BUILD_MSG[]; //=            "Print from SD";
extern const PROGMEM prog_uchar PREHEAT_MSG[]; //=          "Preheat";
extern const PROGMEM prog_uchar UTILITIES_MSG[]; //=        "Utilities";
extern const PROGMEM prog_uchar MONITOR_MSG[]; //=          "Monitor Mode";
extern const PROGMEM prog_uchar JOG_MSG[]; //  =            "Jog Mode";
//extern const PROGMEM prog_uchar CALIBRATION_MSG[]; //=      "Calibrate Axes";
extern const PROGMEM prog_uchar HOME_AXES_MSG[]; //=        "Home Axes";
extern const PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[]; //= "Filament Loading";
extern const PROGMEM prog_uchar VERSION_MSG[]; //= 	       "Version Information";
extern const PROGMEM prog_uchar DSTEPS_MSG[]; //=           "Disable Steppers";
extern const PROGMEM prog_uchar ESTEPS_MSG[]; //=           "Enable Steppers  ";
extern const PROGMEM prog_uchar PLATE_LEVEL_MSG[]; //=      "Level Build Plate";
extern const PROGMEM prog_uchar PREHEAT_SET_MSG[]; //=      "Preheat Settings";
extern const PROGMEM prog_uchar SETTINGS_MSG[]; //=         "General Settings";
extern const PROGMEM prog_uchar RESET_MSG[]; //=            "Restore Settings";
extern const PROGMEM prog_uchar NOZZLES_MSG[]; //=          "Calibrate Nozzles";
extern const PROGMEM prog_uchar BOT_STATS_MSG[]; //=        "Bot Statistics";
extern const PROGMEM prog_uchar TOOL_COUNT_MSG[]; //=       "Extruders";
extern const PROGMEM prog_uchar SOUND_MSG[]; //=            "Sound";
//extern const PROGMEM prog_uchar LED_MSG[]; //=              "LED Colour";
//extern const PROGMEM prog_uchar LED_HEAT_MSG[]; //=         "Heat LEDs";
extern const PROGMEM prog_uchar EXIT_MSG[]; //=             "Exit menu";
extern const PROGMEM prog_uchar ACCELERATE_MSG[]; //= 	     "Accelerate";
extern const PROGMEM prog_uchar HBP_MSG[]; //                = "HBP installed";
extern const PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[]; //= "Override GcTemp";
extern const PROGMEM prog_uchar PAUSE_HEAT_MSG[];  //        = "Pause with Heat";
extern const PROGMEM prog_uchar EXTRUDER_HOLD_MSG[]; //      = "Extruder Hold";
extern const PROGMEM prog_uchar SD_USE_CRC_MSG[]; //         = "Check SD reads";
extern const PROGMEM prog_uchar PSTOP_ENABLE_MSG[]; //       = "P-Stop control";
extern const PROGMEM prog_uchar DISABLED_MSG[]; //           = "N/A";
#ifdef DITTO_PRINT
extern const PROGMEM prog_uchar DITTO_PRINT_MSG[]; //        = "Ditto Printing";
#endif
extern const PROGMEM prog_uchar PAUSEATZPOS_MSG[];  //        = "Pause at ZPos";
extern const PROGMEM prog_uchar CHANGE_SPEED_MSG[]; //       = "Change Speed";
extern const PROGMEM prog_uchar CHANGE_TEMP_MSG[]; //        = "Change Temperature";
extern const PROGMEM prog_uchar FAN_ON_MSG[]; //             = "Set Cooling Fan ON "; // Needs trailing space
extern const PROGMEM prog_uchar FAN_OFF_MSG[]; //            = "Set Cooling Fan OFF";
extern const PROGMEM prog_uchar RETURN_TO_MAIN_MSG[]; //     = "Main Menu";
extern const PROGMEM prog_uchar PRINT_ANOTHER_MSG[]; //      = "Print Another Copy";
extern const PROGMEM prog_uchar CANNOT_PRINT_ANOTHER_MSG[]; // = "(Cannot Print Copy)";
//
//
//extern const PROGMEM prog_uchar RED_COLOR_MSG[]; //   = "RED   ";
//extern const PROGMEM prog_uchar ORANGE_COLOR_MSG[]; //= "ORANGE";
//extern const PROGMEM prog_uchar PINK_COLOR_MSG[]; //  = "PINK  ";
//extern const PROGMEM prog_uchar GREEN_COLOR_MSG[]; // = "GREEN ";
//extern const PROGMEM prog_uchar BLUE_COLOR_MSG[]; //  = "BLUE  ";
//extern const PROGMEM prog_uchar PURPLE_COLOR_MSG[]; //= "PURPLE";
//extern const PROGMEM prog_uchar WHITE_COLOR_MSG[]; // = "WHITE ";
//extern const PROGMEM prog_uchar CUSTOM_COLOR_MSG[]; //= "CUSTOM";
//extern const PROGMEM prog_uchar OFF_COLOR_MSG[]; //   = "OFF   ";

extern const PROGMEM prog_uchar ERROR_MSG[]; //=   "error!";
extern const PROGMEM prog_uchar NA_MSG[]; //=      "  NA    ";
extern const PROGMEM prog_uchar WAITING_MSG[]; //= "waiting ";

extern const PROGMEM prog_uchar EXTEMP_CHANGE_MSG[]; //              = "My temperature was  " "changed externally. " "Reselect filament   " "menu to try again.";
extern const PROGMEM prog_uchar HEATER_INACTIVITY_MSG[]; //          = "Heaters shutdown due" "to inactivity";
extern const PROGMEM prog_uchar HEATER_TOOL_MSG[]; //                = "Extruder Failure!   ";
extern const PROGMEM prog_uchar HEATER_TOOL0_MSG[]; //               = "Tool 0 Failure!     ";
extern const PROGMEM prog_uchar HEATER_TOOL1_MSG[]; //               = "Tool 1 Failure!     ";
extern const PROGMEM prog_uchar HEATER_PLATFORM_MSG[]; //            = "Platform Failure!   ";
//
extern const PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[]; //= "Temp limit reached. " "Shutdown or restart.";
extern const PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[]; //    = "Not heating properly" "Check wiring.";
extern const PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[]; //  = "Temperature dropping" "Check wiring.";
extern const PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[]; // = "Temperature reads   " "are failing.        " "Check wiring.";
extern const PROGMEM prog_uchar HEATER_FAIL_READ_MSG[]; //           = "Temperature reads   " "out of range.       " "Check wiring.";
//
extern const PROGMEM prog_uchar BUILD_TIME_MSG[];  //= "Print Time:     h  m";
extern const PROGMEM prog_uchar Z_POSITION_MSG[];  //= "ZPosition:";
extern const PROGMEM prog_uchar FILAMENT_MSG[];  //  = "Filament:";
extern const PROGMEM prog_uchar PROFILES_MSG[];  //  = "Profiles";
extern const PROGMEM prog_uchar HOME_OFFSETS_MSG[]; //= "Home Offsets";
extern const PROGMEM prog_uchar MILLIMETERS_MSG[]; //= "mm";
extern const PROGMEM prog_uchar METERS_MSG[];  //  = "m";
extern const PROGMEM prog_uchar LINE_NUMBER_MSG[]; //= "Line:               ";
//extern const PROGMEM prog_uchar LEFT_EXIT_MSG []; // = "Left Key to Go Back ";

extern const PROGMEM prog_uchar BACK_TO_MONITOR_MSG[]; //="Back to Monitor";
extern const PROGMEM prog_uchar STATS_MSG[];  //		= "Print Statistics";
extern const PROGMEM prog_uchar CANCEL_BUILD_MSG[]; //   = "Cancel Print";
extern const PROGMEM prog_uchar HEATERS_OFF_MSG[]; //   = "Heaters Off";

extern const PROGMEM prog_uchar FILAMENT_ODOMETER_MSG[]; //   = "Filament Odometer";
extern const PROGMEM prog_uchar FILAMENT_LIFETIME1_MSG[]; //  = "Life:";
extern const PROGMEM prog_uchar FILAMENT_TRIP1_MSG[]; //      = "Trip:";
extern const PROGMEM prog_uchar FILAMENT_LIFETIME2_MSG[]; //  = "Filament:";
extern const PROGMEM prog_uchar FILAMENT_TRIP2_MSG[]; //  	   = "Fil. Trip:";
extern const PROGMEM prog_uchar FILAMENT_RESET_TRIP_MSG[]; // = "  'M' - Reset Trip";

extern const PROGMEM prog_uchar PROFILE_RESTORE_MSG[];  //	= "Restore";
extern const PROGMEM prog_uchar PROFILE_DISPLAY_CONFIG_MSG[];  //= "Display Config";
extern const PROGMEM prog_uchar PROFILE_CHANGE_NAME_MSG[];  //= "Change Name";
extern const PROGMEM prog_uchar PROFILE_SAVE_TO_PROFILE_MSG[];  //= "Save To Profile";
extern const PROGMEM prog_uchar PROFILE_PROFILE_NAME_MSG[];  //= "Profile Name:";
extern const PROGMEM prog_uchar UPDNLRM_MSG[];  //		= "Up/Dn/R/L/M to Set";
extern const PROGMEM prog_uchar UPDNLM_MSG[];  //		= "Up/Dn/Left/M to Set";
extern const PROGMEM prog_uchar XYZOFFSET_MSG[]; //                = " Offset: ";  // needs extra space
extern const PROGMEM prog_uchar PROFILE_RIGHT_MSG[];  //	= "Right Temp: ";
extern const PROGMEM prog_uchar PROFILE_LEFT_MSG[];  //	= "Left Temp: ";
extern const PROGMEM prog_uchar PROFILE_PLATFORM_MSG[];  //	= "Platform Temp:   ";
extern const PROGMEM prog_uchar PAUSE_AT_ZPOS_MSG[];  //	= "Pause at ZPos: ";

extern const PROGMEM prog_uchar PRINTED_TOO_LONG_MSG[];  //	= "Line:    1000000000+";

extern const PROGMEM prog_uchar PAUSE_ENTER_MSG[];  //		= "Entering Pause...   "; // Needs trailing spaces
extern PROGMEM const prog_uchar CANCELLING_ENTER_MSG[]; //		= "Cancelling Print... "; // Needs trailing spaces
extern const PROGMEM prog_uchar PAUSE_LEAVE_MSG[];  //		= "Leaving Pause...    "; // Needs trailing spaces
extern const PROGMEM prog_uchar PAUSE_DRAINING_PIPELINE_MSG[];  //	= "Draining pipeline..."; // Needs trailing spaces
extern const PROGMEM prog_uchar PAUSE_CLEARING_BUILD_MSG[]; //	= "Clearing build...   "; // Needs trailing spaces
extern const PROGMEM prog_uchar PAUSE_RESUMING_POSITION_MSG[];  //	= "Resuming position..."; // Needs trailing spaces
extern const PROGMEM prog_uchar TOTAL_TIME_MSG[]; //               = "Lifetime:      h 00m";
extern const PROGMEM prog_uchar LAST_TIME_MSG[]; //                = "Last Print:    h 00m";
extern const PROGMEM prog_uchar BUILD_TIME2_MSG[]; //              =  "Print Time:   h 00m"; // This string is 19 chars WIDE!
#ifdef EEPROM_MENU_ENABLE
extern const PROGMEM prog_uchar EEPROM_MSG[];           // = "Eeprom";
extern const PROGMEM prog_uchar EEPROM_DUMP_MSG[];      // = "Eeprom -> SD";
extern const PROGMEM prog_uchar EEPROM_RESTORE_MSG[];   // = "SD -> Eeprom";
extern const PROGMEM prog_uchar EEPROM_ERASE_MSG[];     // = "Erase Eeprom";
#endif

extern const PROGMEM prog_uchar ERROR_STREAM_VERSION[]; //= "This is not the x3g version I work best with. "  "For help see: makerbot.com/help   ";

#ifdef MODEL_REPLICATOR2
extern const PROGMEM prog_uchar ERROR_BOT_TYPE[]; //= "I am a Replicator 2.This build is for   another bot. See:   makerbot.com/help";
#else
extern const PROGMEM prog_uchar ERROR_BOT_TYPE[]; //= "I am a Replicator.  This build is for   another bot. See:   makerbot.com/help";
#endif

#endif // __LOCALE_HH_INCLUDED__

