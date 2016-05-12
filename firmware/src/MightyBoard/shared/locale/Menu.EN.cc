#include "Model.hh"
#include "locale.hh"

const PROGMEM prog_uchar ON_MSG[] =      "ON ";
const PROGMEM prog_uchar OFF_MSG[] =     "OFF";

//#ifdef STACK_PAINT
const PROGMEM prog_uchar SPLASH_SRAM_MSG[] = "Free SRAM ";
//#endif

const PROGMEM prog_uchar SPLASH1A_MSG[] = "    FAIL!           ";
const PROGMEM prog_uchar SPLASH2A_MSG[] = "    SUCCESS!        ";
const PROGMEM prog_uchar SPLASH3A_MSG[] = "connected correctly ";
const PROGMEM prog_uchar SPLASH4A_MSG[] = "Heaters are not     ";

const PROGMEM prog_uchar GO_MSG[] =         "Start Preheating";
const PROGMEM prog_uchar STOP_MSG[] =       "Turn Heaters OFF";
const PROGMEM prog_uchar PLATFORM_MSG[] =   "Platform";
const PROGMEM prog_uchar TOOL_MSG[] =       "Extruder";
const PROGMEM prog_uchar START_TEST_MSG[]=  "I'm going to print  " "a series of lines so" "we can find my      " "nozzle alignment.   ";
const PROGMEM prog_uchar EXPLAIN1_MSG[] =   "Look for the best   " "matched line in each" "axis set.  Lines are" "numbered 1-13 and...";
const PROGMEM prog_uchar EXPLAIN2_MSG[] =   "line one is extra   " "long. The Y axis set" "is left on the plate" "and X axis is right.";
const PROGMEM prog_uchar END_MSG  [] =      "Great!  I've saved  " "these settings and  " "I'll use them to    " "make nice prints!   ";

const PROGMEM prog_uchar SELECT_MSG[] =     "Select best lines:";
const PROGMEM prog_uchar DONE_MSG[]   =     "Done";

const PROGMEM prog_uchar NO_MSG[]   =       "NO ";     // length 3
const PROGMEM prog_uchar YES_MSG[]  =       "YES";     // length 3
const uint8_t YES_NO_WIDTH = 3;

const PROGMEM prog_uchar XAXIS_MSG[] =      "X Axis Line";
const PROGMEM prog_uchar YAXIS_MSG[] =      "Y Axis Line";

const PROGMEM prog_uchar HEATER_ERROR_MSG[]=  "Extruders are not   " "heating.  Check the " "wiring.";

const PROGMEM prog_uchar STOP_EXIT_MSG[] = "Begin loading or    " "unloading. Press the" "center button when  " "finished.";
const PROGMEM prog_uchar TIMEOUT_MSG[] = "5 minute timeout has" "elapsed.  Press the " "center button to    " "finish.";

const PROGMEM prog_uchar LOAD_RIGHT_MSG[]    = "Load right";
const PROGMEM prog_uchar LOAD_LEFT_MSG[]     = "Load left";
const PROGMEM prog_uchar LOAD_SINGLE_MSG[]   = "Load";
const PROGMEM prog_uchar UNLOAD_SINGLE_MSG[] = "Unload";
const PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]  = "Unload right";
const PROGMEM prog_uchar UNLOAD_LEFT_MSG[]   = "Unload left";

const PROGMEM prog_uchar JOG1_MSG[]  = "     Jog mode       ";
const PROGMEM prog_uchar JOG2X_MSG[] = "        X+          ";
const PROGMEM prog_uchar JOG3X_MSG[] = "      (Menu)  Y->   ";
const PROGMEM prog_uchar JOG4X_MSG[] = "        X-          ";
const PROGMEM prog_uchar JOG3Y_MSG[] = " <-X  (Menu)  Z->   ";
const PROGMEM prog_uchar JOG3Z_MSG[] = " <-Y  (Menu)        ";


const PROGMEM prog_uchar DISTANCESHORT_MSG[] = "SHORT";
const PROGMEM prog_uchar DISTANCELONG_MSG[] =  "LONG";

#define HEATING_MSG_DEF "Heating:"
const PROGMEM prog_uchar HEATING_MSG[] =        HEATING_MSG_DEF;
// HEATING_SPACES_MSG is a full LCD screen wide (20 spaces)
const PROGMEM prog_uchar HEATING_SPACES_MSG[] = HEATING_MSG_DEF "            ";

const PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
const PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "R Extruder: ---/---C";
const PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "L Extruder: ---/---C";
const PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Platform:   ---/---C";
const PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Extruder:   ---/---C";
const PROGMEM prog_uchar FILAMENT_CANCEL_MSG[] =  "Press left to cancel";


const PROGMEM prog_uchar EXTRUDER_SPACES_MSG[]  = "Extruder           ";
const PROGMEM prog_uchar RIGHT_SPACES_MSG[]     = "Right Extruder     ";
const PROGMEM prog_uchar LEFT_SPACES_MSG[]      = "Left Extruder      ";
const PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Platform           ";
const PROGMEM prog_uchar RESET1_MSG[]           = "Restore factory";
const PROGMEM prog_uchar RESET2_MSG[]           = "settings?";
const PROGMEM prog_uchar CANCEL_MSG[] = "Cancel this print?";
const PROGMEM prog_uchar CANCEL_FIL_MSG[] = "Cancel load/unload?";

const PROGMEM prog_uchar PAUSE_MSG[]      = "Pause  "; // must be same length as the next msg
const PROGMEM prog_uchar UNPAUSE_MSG[]    = "Unpause"; // must be same length as the prior msg
const PROGMEM prog_uchar COLD_PAUSE_MSG[] = "Cold Pause";

#if defined(DEBUG_SD)
const PROGMEM prog_uchar CARD1_MSG[]    = "SD #1: Comms";
const PROGMEM prog_uchar CARD2_MSG[]    = "SD #2: CRC";
const PROGMEM prog_uchar CARD3_MSG[]    = "SD #3: Voltage";
const PROGMEM prog_uchar CARD4_MSG[]    = "SD #4: Pattern";
const PROGMEM prog_uchar CARD5_MSG[]    = "SD #5: Bad Resp";
const PROGMEM prog_uchar CARD6_MSG[]    = "SD #6: FOO";
const PROGMEM prog_uchar CARD7_MSG[]    = "SD #7: FOO2";
#endif

const PROGMEM prog_uchar BUILDING_MSG[]    = "I'm already building";
const PROGMEM prog_uchar CARDCOMMS_MSG[]   = "SD card reads are   " "not going well.  Try" "a different card";
const PROGMEM prog_uchar CARDOPENERR_MSG[] = "Cannot open the file";
const PROGMEM prog_uchar CARDNOFILES_MSG[] = "SD card is empty";
const PROGMEM prog_uchar CARDPART_MSG[]    = "Cannot open SD card " "partition";
const PROGMEM prog_uchar CARDROOT_MSG[]    = "SD card lacks a root" "partition";
const PROGMEM prog_uchar CARDINIT_MSG[]    = "Cannot init SD card";
const PROGMEM prog_uchar NOCARD_MSG[]      = "SD card not present";
const PROGMEM prog_uchar CARDERROR_MSG[]   = "SD card read error";
const PROGMEM prog_uchar CARDCRC_MSG[]     = "SD card read error. " "Too many CRC errors." "Bad card contacts or" "electrical noise.";
const PROGMEM prog_uchar CARDFORMAT_MSG[]  = "Unable to read this " "SD card format.     " "Format as FAT-16 or " "FAT-32.";
const PROGMEM prog_uchar CARDSIZE_MSG[]    = "Unable to read SD   " "card partitions over" "2GB in size.  Format" "as FAT-16 or -32.";
const PROGMEM prog_uchar EXTRUDER_TIMEOUT_MSG[] = "I timed out while   attempting to heat  my extruder.";
const PROGMEM prog_uchar PLATFORM_TIMEOUT_MSG[] = "I timed out while   attempting to heat  my platform.";

const PROGMEM prog_uchar BUILD_MSG[] =            "Print from SD";
const PROGMEM prog_uchar PREHEAT_MSG[] =          "Preheat";
const PROGMEM prog_uchar UTILITIES_MSG[] =        "Utilities";
const PROGMEM prog_uchar MONITOR_MSG[] =          "Monitor Mode";
const PROGMEM prog_uchar JOG_MSG[]   =            "Jog Mode";
const PROGMEM prog_uchar CALIBRATION_MSG[] =      "Calibrate Axes";
const PROGMEM prog_uchar HOME_AXES_MSG[] =        "Home Axes";
const PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Filament Loading";
const PROGMEM prog_uchar VERSION_MSG[] = 	       "Version Information";
const PROGMEM prog_uchar DSTEPS_MSG[] =           "Disable Steppers";
const PROGMEM prog_uchar ESTEPS_MSG[] =           "Enable Steppers  ";
const PROGMEM prog_uchar PLATE_LEVEL_MSG[] =      "Level Build Plate";
const PROGMEM prog_uchar PREHEAT_SET_MSG[] =      "Preheat Settings";
const PROGMEM prog_uchar SETTINGS_MSG[] =         "General Settings";
const PROGMEM prog_uchar RESET_MSG[] =            "Restore Settings";
const PROGMEM prog_uchar NOZZLES_MSG[] =          "Calibrate Nozzles";
const PROGMEM prog_uchar BOT_STATS_MSG[] =        "Bot Statistics";
const PROGMEM prog_uchar TOOL_COUNT_MSG[] =       "Extruder Count";
const PROGMEM prog_uchar SOUND_MSG[] =            "Sound";
//const PROGMEM prog_uchar LED_HEAT_MSG[] =         "Heat LEDs";
const PROGMEM prog_uchar EXIT_MSG[] =             "Exit Menu";
const PROGMEM prog_uchar ACCELERATE_MSG[]          = "Acceleration";
const PROGMEM prog_uchar HBP_MSG[]                 = "HBP installed";
const PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[] = "Override GcTemp";
const PROGMEM prog_uchar PAUSE_HEAT_MSG[]	   = "Pause with Heat";
const PROGMEM prog_uchar EXTRUDER_HOLD_MSG[]       = "Extruder Hold";
const PROGMEM prog_uchar SD_USE_CRC_MSG[]          = "Check SD Reads";
#if defined(PSTOP_SUPPORT)
#if defined(ZYYX_3D_PRINTER)
const PROGMEM prog_uchar PSTOP_ENABLE_MSG[]        = "Filament Sensor";
const PROGMEM prog_uchar PSTOP_INVERTED_MSG[]      = "Sensor Inverted";
const PROGMEM prog_uchar PSTOP_MSG[]               = "Filament problem    " "detected. Check the " "filament spool.     " "LEFT for menu.";
#else
const PROGMEM prog_uchar PSTOP_ENABLE_MSG[]        = "P-Stop Control";
const PROGMEM prog_uchar PSTOP_INVERTED_MSG[]      = "P-Stop Inverted";
const PROGMEM prog_uchar PSTOP_MSG[]               = "P-Stop triggered";
#endif
#endif
const PROGMEM prog_uchar DISABLED_MSG[]            = "N/A";
//#ifdef DITTO_PRINT
const PROGMEM prog_uchar DITTO_PRINT_MSG[]         = "Ditto Printing";
//#endif
const PROGMEM prog_uchar PAUSEATZPOS_MSG[]	   = "Pause at ZPos";
const PROGMEM prog_uchar CHANGE_SPEED_MSG[]        = "Change Speed";
const PROGMEM prog_uchar CHANGE_TEMP_MSG[]         = "Change Temperature";
const PROGMEM prog_uchar CHANGE_HBP_TEMP_MSG[]     = "Change HBP Temp";
const PROGMEM prog_uchar FAN_ON_MSG[]              = "Set Cooling Fan ON "; // Needs trailing space
const PROGMEM prog_uchar FAN_OFF_MSG[]             = "Set Cooling Fan OFF";
const PROGMEM prog_uchar RETURN_TO_MAIN_MSG[]      = "Main Menu";
const PROGMEM prog_uchar PRINT_ANOTHER_MSG[]       = "Print Another Copy";
const PROGMEM prog_uchar CANNOT_PRINT_ANOTHER_MSG[]= "(Cannot Print Copy)";

#if defined(HAS_RGB_LED)
const PROGMEM prog_uchar LIGHTS_OFF_MSG[]   = "Set Lights OFF";
const PROGMEM prog_uchar LIGHTS_ON_MSG[]    = "Set Lights ON ";
#if defined(RGB_LED_MENU)
const PROGMEM prog_uchar LED_MSG[]          = "LED Colour";
const PROGMEM prog_uchar RED_COLOR_MSG[]    = "RED   ";
const PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE";
const PROGMEM prog_uchar PINK_COLOR_MSG[]   = "PINK  ";
const PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "GREEN ";
const PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLUE  ";
const PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "PURPLE";
const PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "WHITE ";
const PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "CUSTOM";
const PROGMEM prog_uchar OFF_COLOR_MSG[]    = "OFF   ";
#endif
#endif

const PROGMEM prog_uchar ERROR_MSG[] =   "error!";
const PROGMEM prog_uchar NA_MSG[] =      "  NA    ";
const PROGMEM prog_uchar WAITING_MSG[] = "waiting ";

const PROGMEM prog_uchar EXTEMP_CHANGE_MSG[]               = "My temperature was  " "changed externally. " "Reselect filament   " "menu to try again.";
const PROGMEM prog_uchar FILCANCEL_MSG[]                   = "Filament load/unload" "canceled due to the " "prior heater error";
const PROGMEM prog_uchar HEATER_INACTIVITY_MSG[]           = "Heaters shutdown due" "to inactivity";
const PROGMEM prog_uchar HEATER_TOOL_MSG[]                 = "Extruder Failure!   ";
const PROGMEM prog_uchar HEATER_TOOL0_MSG[]                = "Tool 0 Failure!     ";
const PROGMEM prog_uchar HEATER_TOOL1_MSG[]                = "Tool 1 Failure!     ";
const PROGMEM prog_uchar HEATER_PLATFORM_MSG[]             = "Platform Failure!   ";

const PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[] = "Temp limit reached. " "Shutdown or restart.";
const PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[]     = "Not heating properly" "Check wiring.";
const PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[]   = "Temperature dropping" "Check wiring.";
const PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[]  = "Temperature reads   " "are failing.        " "Check wiring.";
const PROGMEM prog_uchar HEATER_FAIL_READ_MSG[]            = "Temperature reads   " "out of range.       " "Check wiring.";

const PROGMEM prog_uchar BUILD_TIME_MSG[]	= "Print Time:     h  m";

// Spaces needed at end to deal with alternation with auto-level status
const PROGMEM prog_uchar Z_POSITION_MSG[]	= "ZPosition:   ";

const PROGMEM prog_uchar FILAMENT_MSG[]         = "Filament:";
const PROGMEM prog_uchar MON_FILAMENT_MSG[]     = "Filament:      0.0mm";
const PROGMEM prog_uchar PROFILES_MSG[]	  = "Profiles";
const PROGMEM prog_uchar HOME_OFFSETS_MSG[]= "Home Offsets";
const PROGMEM prog_uchar MILLIMETERS_MSG[] = "mm";
const PROGMEM prog_uchar METERS_MSG[]	  = "m";
const PROGMEM prog_uchar LINE_NUMBER_MSG[] = "Line:               ";
const PROGMEM prog_uchar LEFT_EXIT_MSG []  = "Left Key to Go Back ";

const PROGMEM prog_uchar BACK_TO_MONITOR_MSG[] ="Back to Monitor";
const PROGMEM prog_uchar STATS_MSG[]			= "Print Statistics";
const PROGMEM prog_uchar CANCEL_BUILD_MSG[]    = "Cancel Print";
const PROGMEM prog_uchar HEATERS_OFF_MSG[]    = "Heaters Off";

const PROGMEM prog_uchar FILAMENT_ODOMETER_MSG[]    = "Filament Odometer";
const PROGMEM prog_uchar FILAMENT_LIFETIME1_MSG[]   = "Life:";
const PROGMEM prog_uchar FILAMENT_TRIP1_MSG[]       = "Trip:";
const PROGMEM prog_uchar FILAMENT_LIFETIME2_MSG[]   = "Filament:";
const PROGMEM prog_uchar FILAMENT_TRIP2_MSG[]       = "Fil. Trip:";
const PROGMEM prog_uchar FILAMENT_RESET_TRIP_MSG[]  = "Reset: press CENTER";

const PROGMEM prog_uchar PROFILE_RESTORE_MSG[]		= "Restore";
const PROGMEM prog_uchar PROFILE_DISPLAY_CONFIG_MSG[]	= "Display Config";
const PROGMEM prog_uchar PROFILE_CHANGE_NAME_MSG[]	= "Change Name";
const PROGMEM prog_uchar PROFILE_SAVE_TO_PROFILE_MSG[]	= "Save To Profile";
const PROGMEM prog_uchar PROFILE_PROFILE_NAME_MSG[]	= "Profile Name:";
const PROGMEM prog_uchar UPDNLRM_MSG[]			= "Up/Dn/R/L/M to set";
const PROGMEM prog_uchar UPDNLM_MSG[]			= "Up/Dn/Left/M to set";
const PROGMEM prog_uchar XYZOFFSET_MSG[]                = " Offset:";  // needs extra space
const PROGMEM prog_uchar XYZTOOLHEAD_MSG[]              = " Toolhead Offset:"; // needs extra space
const PROGMEM prog_uchar TOOLHEAD_OFFSETS_MSG[]         = "Toolhead Offsets";
const PROGMEM prog_uchar PROFILE_RIGHT_MSG[]		= "Right Temp: ";
const PROGMEM prog_uchar PROFILE_LEFT_MSG[]		= "Left Temp: ";
const PROGMEM prog_uchar PROFILE_PLATFORM_MSG[]		= "Platform Temp:   ";
const PROGMEM prog_uchar PAUSE_AT_ZPOS_MSG[]		= "Pause at ZPos: ";

const PROGMEM prog_uchar PRINTED_TOO_LONG_MSG[]		= "Line:    1000000000+";

const PROGMEM prog_uchar PAUSE_ENTER_MSG[]		= "Entering pause...   "; // Needs trailing spaces
const PROGMEM prog_uchar CANCELLING_ENTER_MSG[]		= "Cancelling print... "; // Needs trailing spaces
const PROGMEM prog_uchar PAUSE_LEAVE_MSG[]		= "Leaving pause...    "; // Needs trailing spaces
const PROGMEM prog_uchar PAUSE_DRAINING_PIPELINE_MSG[]	= "Draining pipeline..."; // Needs trailing spaces
const PROGMEM prog_uchar PAUSE_CLEARING_BUILD_MSG[]	= "Clearing build...   "; // Needs trailing spaces
const PROGMEM prog_uchar PAUSE_RESUMING_POSITION_MSG[]	= "Resuming position..."; // Needs trailing spaces
const PROGMEM prog_uchar TOTAL_TIME_MSG[]                = "Lifetime:      h 00m";
const PROGMEM prog_uchar LAST_TIME_MSG[]                 = "Last Print:    h 00m";
const PROGMEM prog_uchar BUILD_TIME2_MSG[]               =  "Print Time:   h 00m"; // This string is 19 chars WIDE!

#if defined(EEPROM_MENU_ENABLE)
const PROGMEM prog_uchar EEPROM_MSG[]		= "Eeprom";
const PROGMEM prog_uchar EEPROM_DUMP_MSG[]	= "Eeprom -> SD";
const PROGMEM prog_uchar EEPROM_RESTORE_MSG[]	= "SD -> Eeprom";
const PROGMEM prog_uchar EEPROM_ERASE_MSG[]	= "Erase Eeprom";
#endif

#if defined(ALTERNATE_UART)
const PROGMEM prog_uchar ALT_UART_MSG[] = "Serial I/O";
const PROGMEM prog_uchar ALT_UART_0_MSG[] = "  USB";
const PROGMEM prog_uchar ALT_UART_1_MSG[] = "UART1";
#endif

#if defined(AUTO_LEVEL)
const PROGMEM prog_uchar ALEVEL_UTILITY_COMP_MSG[] = "Auto-level Adj";
const PROGMEM prog_uchar ALEVEL_COMP_OFFSET_MSG[]  = " Deflection"; // needs extra space
const PROGMEM prog_uchar ALEVEL_UTILITY_MSG[]    = "Auto-level Variance"; // No more than 19 characters
const PROGMEM prog_uchar ALEVEL_SCREEN_MSG1[]    = "Max height variance";  // No more than 19 characters
const PROGMEM prog_uchar ALEVEL_SCREEN_MSG2[]    = "between probe pts."; // No more than 19 characters
const PROGMEM prog_uchar ALEVEL_COLINEAR_MSG[]   = "Auto-level failed\nBad probing points";
const PROGMEM prog_uchar ALEVEL_INCOMPLETE_MSG[] = "Auto-level failed\nIncomplete probing";
const PROGMEM prog_uchar ALEVEL_INACTIVE_MSG[]   = "Auto-level inactive ";  // must be 20 chars
const PROGMEM prog_uchar ALEVEL_ACTIVE_MSG[]     = "Auto-level  0.000 mm";  // must be 20 chars
const PROGMEM prog_uchar ALEVEL_GOOD_MSG[]       = "The build plate is  " "leveled!";
const PROGMEM prog_uchar ALEVEL_FAIL_MSG[]       = "The build plate is  " "not leveled!";
const PROGMEM prog_uchar ALEVEL_MSG2[]           = "Press CENTER button " "to continue.";
#if defined(ZYYX_3D_PRINTER)
const PROGMEM prog_uchar ALEVEL_BADLEVEL_MSG[]   =  "Auto-level failed   " "Check plate or run  " "the leveling script.";
const PROGMEM prog_uchar ALEVEL_NOT_CALIBRATED_MSG[] = "Auto-level failed\nProbe not calibrated";
#else
const PROGMEM prog_uchar ALEVEL_BADLEVEL_MSG[]   = "Auto-level failed\nToo far out of level";
const PROGMEM prog_uchar ALEVEL_NOT_CALIBRATED_MSG[] = "Auto-level failed\nProbe not calibrated";
#endif
#if defined(PSTOP_SUPPORT) && defined(PSTOP_ZMIN_LEVEL)
#if defined(ZYYX_3D_PRINTER)
const PROGMEM prog_uchar MAX_PROBE_HITS_STOP_MSG[] = "Print obstruction   " "detected.  Check the" "print and extruder. " "Press LEFT for menu.";
const PROGMEM prog_uchar MAX_PROBE_HITS_MSG1[]     = "Total trig. time(s)"; // No more than 19 characters
#else
const PROGMEM prog_uchar MAX_PROBE_HITS_STOP_MSG[] = "Print obstruction   " "detected.  Check the" "print and extruder. " "Press LEFT for menu.";
const PROGMEM prog_uchar MAX_PROBE_HITS_MSG1[]     = "Max Z Probe Hits"; // No more than 19 characters
#endif
const PROGMEM prog_uchar MAX_PROBE_HITS_MSG2[]      = "Use 0 for unlimited";
#endif
#endif

#if defined(ESTIMATE_TIME)
const PROGMEM prog_uchar MON_TIME_LEFT_MSG[]      = "Time Left:   000h00m";
const PROGMEM prog_uchar MON_TIME_LEFT_SECS_MSG[] = " secs";
#endif

#if defined(BUILD_STATS)
const PROGMEM prog_uchar MON_ELAPSED_TIME_MSG[]   = "Elapsed:     000h00m";
#endif

#if defined(MACHINE_ID_MENU)
const PROGMEM prog_uchar MACHINE_ID_MSG[]       = "Bot Type";
const PROGMEM prog_uchar MACHINE_ID_REP1_MSG[]  = "Rep 1";
const PROGMEM prog_uchar MACHINE_ID_REP2_MSG[]  = "Rep 2";
const PROGMEM prog_uchar MACHINE_ID_REP2X_MSG[] = "Rep2X";
#endif

#if defined(COOLING_FAN_PWM)
const PROGMEM prog_uchar COOLING_FAN_PWM_MSG[] = "Cooling Fan Power";
#endif

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3
const PROGMEM prog_uchar CHOOSE_EXT0_THERM_MSG[] = "Tool 0 temp sensor"; 
const PROGMEM prog_uchar CHOOSE_EXT1_THERM_MSG[] = "Tool 1 temp sensor"; 
const PROGMEM prog_uchar CHOOSE_HBP_THERM_MSG[]  = "Bed temp sensor";
const PROGMEM prog_uchar CHOOSE_THERM_MSG[]      = "Temp Sensor Types";
#endif
