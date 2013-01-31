#ifndef __MENU__LOCALES__
#define __MENU__LOCALES__

#include <avr/pgmspace.h>
#include <string.h>

#define FRENCH 1

const static PROGMEM prog_uchar ON_MSG[] =      "ON ";
const static PROGMEM prog_uchar OFF_MSG[] =     "OFF";

const static PROGMEM prog_uchar ON_CELCIUS_MSG[] = "/   C";
const static PROGMEM prog_uchar CELCIUS_MSG[] =    "C    ";
const static PROGMEM prog_uchar BLANK_CHAR_MSG[] = " ";
const static PROGMEM prog_uchar BLANK_CHAR_4_MSG[] = "    ";

const static PROGMEM prog_uchar CLEAR_MSG[] =  "                    ";
const static PROGMEM prog_uchar BLANKLINE_MSG[] =  "                ";



#ifdef LOCALE_FR
#include "Menu.FR.hh"
#else // Use US ENGLISH as default

#define LEVEL_PLATE const static uint8_t LevelPlate[] PROGMEM = { 137,  16,  153,  0,  0,  0,  0,  82,  101,  112,  71,  32,  66,  117,  105,  108,  100,  0,  150,  0,  255,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  155,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  165,  28,  0,  0,  24,  0,  0,  160,  64,  149,  4,  131,  4,  220,  5,  0,  0,  20,  0,  144,  31,  139,  0,  0,  0,  0,  0,  0,  0,  0,  160,  15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  0,  0,  0,  155,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  165,  28,  0,  0,  24,  0,  0,  32,  65,  149,  4,  137,  27,  149,  6,  0,  0,  0,  80,  114,  101,  115,  115,  32,  34,  77,  34,  32,  119,  104,  101,  110,  32,  108,  101,  118,  101,  108,  0,  137,  31,  137,  31 };

#define LEVEL_PLATE_LEN 189


#ifdef MODEL_REPLICATOR
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish Replicator1";
#if !defined(HEATERS_ON_STEROIDS)
const static PROGMEM prog_uchar SPLASH2_MSG[] = "--------------------";
#else
const static PROGMEM prog_uchar SPLASH2_MSG[] = "-- Heater Special --";
#endif
#elif MODEL_REPLICATOR2
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish Replicator2";
const static PROGMEM prog_uchar SPLASH12_MSG[] = "  Sailfish Rep 2X   ";
const static PROGMEM prog_uchar SPLASH2_MSG[] = "--------------------";
#else
#warning "*** Compiling without MODEL_x defined ***"
const static PROGMEM prog_uchar SPLASH1_MSG[] = "      Sailfish      ";
const static PROGMEM prog_uchar SPLASH2_MSG[] = "      --------      ";
#endif
const static PROGMEM prog_uchar SPLASH3_MSG[] = "Thing: 32084 (r     ";
const static PROGMEM prog_uchar SPLASH4_MSG[] = "Firmware Version 7. ";
const static PROGMEM prog_uchar SPLASH5_MSG[] = "Internal Release    ";

const static PROGMEM prog_uchar SPLASH1A_MSG[] = "    FAIL!           ";
const static PROGMEM prog_uchar SPLASH2A_MSG[] = "    SUCCESS!        ";
const static PROGMEM prog_uchar SPLASH3A_MSG[] = "connected correctly ";
const static PROGMEM prog_uchar SPLASH4A_MSG[] = "Heaters are not     ";
const static PROGMEM prog_uchar SPLASH5A_MSG[] = "                    ";	

const static PROGMEM prog_uchar GO_MSG[] =         "Start Preheat!";
const static PROGMEM prog_uchar STOP_MSG[] =       "Cool!";
const static PROGMEM prog_uchar RIGHT_TOOL_MSG[] = "Right Tool";
const static PROGMEM prog_uchar LEFT_TOOL_MSG[] =  "Left Tool";
const static PROGMEM prog_uchar PLATFORM_MSG[] =   "Platform";
const static PROGMEM prog_uchar TOOL_MSG[] =       "Extruder";

const static PROGMEM prog_uchar START_TEST_MSG[]=  "I'm going to print  " "a series of lines so" "we can find my      " "nozzle alignment.   "; // XXX old name: start[]
const static PROGMEM prog_uchar EXPLAIN1_MSG[] =   "Look for the best   " "matched line in each" "axis set. Lines are " "numbered 1-13 and...";
const static PROGMEM prog_uchar EXPLAIN2_MSG[] =   "line one is extra   " "long. The Y axis set" "is left on the plate" "and X axis is right.";
const static PROGMEM prog_uchar END_MSG  [] =      "Great!  I've saved  " "these settings and  " "I'll use them to    " "make nice prints!   ";

const static PROGMEM prog_uchar SELECT_MSG[] =     "Select best lines.";
const static PROGMEM prog_uchar DONE_MSG[]   =     "Done!";
const static PROGMEM prog_uchar NO_MSG[]   =       "No";
const static PROGMEM prog_uchar YES_MSG[]  =       "Yes";

const static PROGMEM prog_uchar XAXIS_MSG[] =      "X Axis Line";
const static PROGMEM prog_uchar YAXIS_MSG[] =      "Y Axis Line";

const static PROGMEM prog_uchar HEATER_ERROR_MSG[]=  "My extruders are    " "not heating up.     " "Check my            " "connections!        ";

const static PROGMEM prog_uchar HEATING_BAR_MSG[] = "I'm heating up my   " "extruder!           " "Please wait...      " "                    ";
const static PROGMEM prog_uchar HEATING_PROG_MSG[]= "Heating Progress:   " "                    " "                    " "                    ";
const static PROGMEM prog_uchar STOP_EXIT_MSG[]   = "You may now load or " "unload the filament." "When you are done,  " "press 'M' to exit.  "; 
const static PROGMEM prog_uchar TIMEOUT_MSG[]  	  = "My motor timed out  " "after 5 minutes.    " "Press M to exit.    " "                    ";

const static PROGMEM prog_uchar LOAD_RIGHT_MSG[] = "Load right"; 
const static PROGMEM prog_uchar LOAD_LEFT_MSG[] =  "Load left"; 
const static PROGMEM prog_uchar LOAD_SINGLE_MSG[] ="Load"; 
const static PROGMEM prog_uchar UNLOAD_SINGLE_MSG[]="Unload"; 
const static PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]="Unload right"; 
const static PROGMEM prog_uchar UNLOAD_LEFT_MSG[] ="Unload left"; 

const static PROGMEM prog_uchar JOG1_MSG[]  = "     Jog mode       ";
const static PROGMEM prog_uchar JOG2X_MSG[] = "        X+          ";
const static PROGMEM prog_uchar JOG3X_MSG[] = "      (Back)   Y->  ";
const static PROGMEM prog_uchar JOG4X_MSG[] = "        X-          ";
const static PROGMEM prog_uchar JOG2Y_MSG[] = "        Y+          ";
const static PROGMEM prog_uchar JOG3Y_MSG[] = "  <-X (Back)  Z->   ";
const static PROGMEM prog_uchar JOG4Y_MSG[] = "        Y-          ";
const static PROGMEM prog_uchar JOG2Z_MSG[] = "        Z-          ";
const static PROGMEM prog_uchar JOG3Z_MSG[] = "  <-Y (Back)        ";
const static PROGMEM prog_uchar JOG4Z_MSG[] = "        Z+          ";


const static PROGMEM prog_uchar DISTANCESHORT_MSG[] = "SHORT";
const static PROGMEM prog_uchar DISTANCELONG_MSG[] =  "LONG";

const static PROGMEM prog_uchar HEATING_MSG[] =        "Heating:";
const static PROGMEM prog_uchar HEATING_SPACES_MSG[] = "Heating:            ";

const static PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
const static PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "R Extruder: ---/---C";
const static PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "L Extruder: ---/---C";
const static PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Platform:   ---/---C";
const static PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Extruder:   ---/---C";


const static PROGMEM prog_uchar PREHEAT_SET_MSG[] = "Preheat Settings    ";
const static PROGMEM prog_uchar RIGHT_SPACES_MSG[] = "Right Tool          ";    // XXX old name: right[]
const static PROGMEM prog_uchar LEFT_SPACES_MSG[]   = "Left Tool           ";   // XXX old name: left[]
const static PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Platform        ";    // XXX old name: platform[]
const static PROGMEM prog_uchar RESET1_MSG[] = "Reset Settings to ";       // XXX old name: set1[]
const static PROGMEM prog_uchar RESET2_MSG[] = "Default values?";
const static PROGMEM prog_uchar CANCEL_MSG[] = "Cancel this print?";
const static PROGMEM prog_uchar CANCEL_PROCESS_MSG[] = "Quit this process?";

const static PROGMEM prog_uchar PAUSE_MSG[] = "Pause    ";
const static PROGMEM prog_uchar UNPAUSE_MSG[] = "UnPause";

const static PROGMEM prog_uchar BUILDING_MSG[] = " I'm already building";
const static PROGMEM prog_uchar CARDOPENERR_MSG[] = "Cannot open the file";
const static PROGMEM prog_uchar CARDNOFILES_MSG[] = "SD card is empty";
const static PROGMEM prog_uchar CARDREMOVED_MSG[] = "SD card Removed";
const static PROGMEM prog_uchar NOCARD_MSG[] = "No SD card found";
const static PROGMEM prog_uchar CARDERROR_MSG[] = "SD card read error";
const static PROGMEM prog_uchar CARDFORMAT_MSG[] = "I can't read this   SD card format!     Try reformatting    the card to FAT16. ";
const static PROGMEM prog_uchar STATICFAIL_MSG[] = "I saw a glitch in mySD card file.SomedayI'll be smart enoughto restart printing.";
const static PROGMEM prog_uchar CARDSIZE_MSG[]   = "I can't read SD 	   cards with storage  larger than 2GB.                      ";
const static PROGMEM prog_uchar EXTRUDER_TIMEOUT_MSG[] = "I timed out while   attempting to heat  my extruder."; 
const static PROGMEM prog_uchar PLATFORM_TIMEOUT_MSG[] = "I timed out while   attempting to heat  my platform.";

const static PROGMEM prog_uchar BUILD_MSG[] =            "Print from SD";
const static PROGMEM prog_uchar PREHEAT_MSG[] =          "Preheat";
const static PROGMEM prog_uchar UTILITIES_MSG[] =        "Utilities";
const static PROGMEM prog_uchar MONITOR_MSG[] =          "Monitor Mode";
const static PROGMEM prog_uchar JOG_MSG[]   =            "Jog Mode";
const static PROGMEM prog_uchar CALIBRATION_MSG[] =      "Calibrate Axes";
const static PROGMEM prog_uchar HOME_AXES_MSG[] =        "Home Axes";
const static PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Filament Options";
const static PROGMEM prog_uchar VERSION_MSG[] = 	 "Version Number";
const static PROGMEM prog_uchar DSTEPS_MSG[] =           "Disable Steppers";
const static PROGMEM prog_uchar ESTEPS_MSG[] =           "Enable Steppers  ";
const static PROGMEM prog_uchar PLATE_LEVEL_MSG[] =      "Level Build Plate";
const static PROGMEM prog_uchar LED_BLINK_MSG[] =        "Blink LEDs       ";
const static PROGMEM prog_uchar LED_STOP_MSG[] =         "Stop Blinking!";
const static PROGMEM prog_uchar PREHEAT_SETTINGS_MSG[] = "Preheat Settings";
const static PROGMEM prog_uchar SETTINGS_MSG[] =         "General Settings";
const static PROGMEM prog_uchar RESET_MSG[] =            "Restore Defaults";
const static PROGMEM prog_uchar NOZZLES_MSG[] =          "Calibrate Nozzles";
const static PROGMEM prog_uchar TOOL_COUNT_MSG[]   =     "Extruders";
const static PROGMEM prog_uchar SOUND_MSG[] =            "Sound";
const static PROGMEM prog_uchar LED_MSG[] =              "LED Color";
const static PROGMEM prog_uchar LED_HEAT_MSG[] =         "Heat LEDs";
const static PROGMEM prog_uchar HELP_SCREENS_MSG[] =     "Help Text";
const static PROGMEM prog_uchar EXIT_MSG[] =             "exit menu";
const static PROGMEM prog_uchar ACCELERATE_MSG[] = 	 "Acceleration";
const static PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[] = "Override GcTemp";
const static PROGMEM prog_uchar PAUSE_HEAT_MSG[]	  = "Pause with Heat";
const static PROGMEM prog_uchar EXTRUDER_HOLD_MSG[]       = "Extruder Hold";
const static PROGMEM prog_uchar TOOL_OFFSET_SYSTEM_MSG[]  = "Tool Offset Sys";
const static PROGMEM prog_uchar OLD_MSG[]                 = "Old";
const static PROGMEM prog_uchar NEW_MSG[]                 = "New";
#ifdef DITTO_PRINT
const static PROGMEM prog_uchar DITTO_PRINT_MSG[]         = "Ditto Printing";
#endif
const static PROGMEM prog_uchar PAUSEATZPOS_MSG[]	 ="Pause at ZPos";

const static PROGMEM prog_uchar RED_COLOR_MSG[]    = "RED   ";
const static PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE";
const static PROGMEM prog_uchar PINK_COLOR_MSG[]   = "PINK  ";
const static PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "GREEN ";
const static PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLUE  ";
const static PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "PURPLE";
const static PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "WHITE ";
const static PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "CUSTOM";

const static PROGMEM prog_uchar RIGHT_MSG[] =   "Right";
const static PROGMEM prog_uchar LEFT_MSG[] =    "Left";
const static PROGMEM prog_uchar ERROR_MSG[] =   "error!";
const static PROGMEM prog_uchar NA_MSG[] =      "  NA    ";
const static PROGMEM prog_uchar WAITING_MSG[] = "waiting ";

const static PROGMEM prog_uchar EXTEMP_CHANGE_MSG[] = "My temperature was  changed externally. Reselect filament   menu to try again.";
const static PROGMEM prog_uchar HEATER_INACTIVITY_MSG[] = "Heaters shutdown    " "due to inactivity   ";
const static PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[] = "Extruder Overheat!  " "Software Temp Limit " "Reached! Please     " "Shutdown or Restart";
const static PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[] = "Heating Failure!    " "My extruders are not" "heating properly.   " "Check my connections";
const static PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[] = "Heating Failure!    " "My extruders are    " "losing temperature. " "Check my connections";
const static PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[] = "Heater Error!       " "My temperature reads" "are failing! Please " "Check my connections";

const static PROGMEM prog_uchar BUILD_TIME_MSG[]	= "Print Time:     h  m"; 
const static PROGMEM prog_uchar Z_POSITION_MSG[]	= "ZPosition:"; 
const static PROGMEM prog_uchar FILAMENT_MSG[]	  = "Filament:"; 
const static PROGMEM prog_uchar PROFILES_MSG[]	  = "Profiles"; 
const static PROGMEM prog_uchar HOME_OFFSETS_MSG[]= "Home Offsets"; 
const static PROGMEM prog_uchar MILLIMETERS_MSG[] = "mm";
const static PROGMEM prog_uchar METERS_MSG[]	  = "m";
const static PROGMEM prog_uchar LINE_NUMBER_MSG[] = "Line:               ";
const static PROGMEM prog_uchar LEFT_EXIT_MSG []  = "Left Key to Go Back ";

const static PROGMEM prog_uchar BACK_TO_MONITOR_MSG[] ="Back to Monitor";
const static PROGMEM prog_uchar STATS_MSG[]			= "Build Statistics";
const static PROGMEM prog_uchar CANCEL_BUILD_MSG[]    = "Cancel Build"; 
const static PROGMEM prog_uchar HEATERS_OFF_MSG[]    = "Heaters Off"; 

const static PROGMEM prog_uchar FILAMENT_ODOMETER_MSG[]    = "Filament Odometer"; 
const static PROGMEM prog_uchar FILAMENT_LIFETIME_MSG[]    = "Life:"; 
const static PROGMEM prog_uchar FILAMENT_TRIP_MSG[]    	   = "Trip:"; 
const static PROGMEM prog_uchar FILAMENT_RESET_TRIP_MSG[]  = "  'M' - Reset Trip"; 

const static PROGMEM prog_uchar PROFILE_RESTORE_MSG[]		= "Restore";
const static PROGMEM prog_uchar PROFILE_DISPLAY_CONFIG_MSG[]	= "Display Config";
const static PROGMEM prog_uchar PROFILE_CHANGE_NAME_MSG[]	= "Change Name";
const static PROGMEM prog_uchar PROFILE_SAVE_TO_PROFILE_MSG[]	= "Save To Profile";
const static PROGMEM prog_uchar PROFILE_PROFILE_NAME_MSG[]	= "Profile Name:";
const static PROGMEM prog_uchar UPDNLRM_MSG[]			= "Up/Dn/R/L/M to Set";
const static PROGMEM prog_uchar UPDNLM_MSG[]			= "Up/Dn/Left/M to Set";
const static PROGMEM prog_uchar XOFFSET_MSG[]			= "X Offset: ";
const static PROGMEM prog_uchar YOFFSET_MSG[]			= "Y Offset: ";
const static PROGMEM prog_uchar ZOFFSET_MSG[]			= "Z Offset: ";
const static PROGMEM prog_uchar PROFILE_RIGHT_MSG[]		= "Right Temp: ";
const static PROGMEM prog_uchar PROFILE_LEFT_MSG[]		= "Left Temp: ";
const static PROGMEM prog_uchar PROFILE_PLATFORM_MSG[]		= "Platform Temp:   ";
const static PROGMEM prog_uchar PAUSE_AT_ZPOS_MSG[]		= "Pause at ZPos: ";

const static PROGMEM prog_uchar PRINTED_TOO_LONG_MSG[]		= "Line:    1000000000+";

const static PROGMEM prog_uchar PAUSE_ENTER_MSG[]		= "Entering Pause:     ";
const static PROGMEM prog_uchar CANCELLING_ENTER_MSG[]		= "Cancelling Print:   ";
const static PROGMEM prog_uchar PAUSE_LEAVE_MSG[]		= "Leaving Pause:      ";
const static PROGMEM prog_uchar PAUSE_DRAINING_PIPELINE_MSG[]	= "Draining pipeline...";
const static PROGMEM prog_uchar PAUSE_CLEARING_BUILD_MSG[]	= "Clearing build...";
const static PROGMEM prog_uchar PAUSE_RESUMING_POSITION_MSG[]	= "Resuming position...";

#ifdef EEPROM_MENU_ENABLE
	const static PROGMEM prog_uchar EEPROM_MSG[]		= "Eeprom";
	const static PROGMEM prog_uchar EEPROM_DUMP_MSG[]	= "Eeprom -> SD";
	const static PROGMEM prog_uchar EEPROM_RESTORE_MSG[]	= "SD -> Eeprom";
	const static PROGMEM prog_uchar EEPROM_ERASE_MSG[]	= "Erase Eeprom";
#endif

const static PROGMEM prog_uchar ERROR_STREAM_VERSION[] = "This is not the x3g version I work best with. "
	"For help see: makerbot.com/help   ";

#ifdef MODEL_REPLICATOR2
const static PROGMEM prog_uchar ERROR_BOT_TYPE[] =
    "I am a Replicator 2.This build is for   another bot. See:   makerbot.com/help";
#else
const static PROGMEM prog_uchar ERROR_BOT_TYPE[] =
    "I am a Replicator.  This build is for   another bot. See:   makerbot.com/help";
#endif

#endif //end of default ELSE for US English */

#endif // __MENU__LOCALES__
