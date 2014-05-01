#ifndef __MENU_EN_HH__
#define __MENU_EN_HH__

const static PROGMEM prog_uchar ON_MSG[] =      "EIN";
const static PROGMEM prog_uchar OFF_MSG[] =     "AUS";

#ifdef STACK_PAINT
const static PROGMEM prog_uchar SPLASH_SRAM_MSG[] = "SRAM frei";
#endif

const static PROGMEM prog_uchar SPLASH1A_MSG[] = "    FAIL!           ";
const static PROGMEM prog_uchar SPLASH2A_MSG[] = "    SUCCESS!        ";
const static PROGMEM prog_uchar SPLASH3A_MSG[] = "connected correctly ";
const static PROGMEM prog_uchar SPLASH4A_MSG[] = "Heaters are not     ";

const static PROGMEM prog_uchar GO_MSG[] =         "Starte vorheizen";
const static PROGMEM prog_uchar STOP_MSG[] =       "Schalte Heiz.aus";
const static PROGMEM prog_uchar PLATFORM_MSG[] =   "Plattform";
const static PROGMEM prog_uchar TOOL_MSG[] =       "Duese";
const static PROGMEM prog_uchar START_TEST_MSG[]=  "Ich drucke eine     " "Serie von Linien,   " "um die Duesen       " "auszurichten.       ";
const static PROGMEM prog_uchar EXPLAIN1_MSG[] =   "Finde die besten    " "uebereinstimmenden  " "Linien je Achse.    " "Linien sind von 1-13";
const static PROGMEM prog_uchar EXPLAIN2_MSG[] =   "Zeile eins          " "ist extra lang.     " "Y Achse ist links   " "X Achse ist rechts. ";
const static PROGMEM prog_uchar END_MSG  [] =      "   Einstellungen    " "       wurden       " "    erfolgreich     " "    gespeichert!    ";

const static PROGMEM prog_uchar SELECT_MSG[] =     "Select best lines.";
const static PROGMEM prog_uchar DONE_MSG[]   =     "Fertig";
const static PROGMEM prog_uchar NO_MSG[]   =       "NEIN";
const static PROGMEM prog_uchar YES_MSG[]  =       "JA ";

const static PROGMEM prog_uchar XAXIS_MSG[] =      "X Achse    ";
const static PROGMEM prog_uchar YAXIS_MSG[] =      "Y Achse    ";

const static PROGMEM prog_uchar HEATER_ERROR_MSG[]=  "Duesen heizen nicht " "Pruefe die Kabel.   " "       ";

const static PROGMEM prog_uchar STOP_EXIT_MSG[]   = "Beginne Laden oder  " "Entladen. Druecke'M'" "zum beenden.";
const static PROGMEM prog_uchar TIMEOUT_MSG[]  	  = "5 Minuten timeout   " "wurden erreicht 'M' " "zum beenden.";

const static PROGMEM prog_uchar LOAD_RIGHT_MSG[]    = "Lade rechts";
const static PROGMEM prog_uchar LOAD_LEFT_MSG[]     = "Lade links";
const static PROGMEM prog_uchar LOAD_SINGLE_MSG[]   = "Lade";
const static PROGMEM prog_uchar UNLOAD_SINGLE_MSG[] = "Entlade";
const static PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]  = "Entlade rechts";
const static PROGMEM prog_uchar UNLOAD_LEFT_MSG[]   = "Entlade links";

const static PROGMEM prog_uchar JOG1_MSG[]  = "      Manuell       ";
const static PROGMEM prog_uchar JOG2X_MSG[] = "        X+          ";
const static PROGMEM prog_uchar JOG3X_MSG[] = "      (Ende)   Y->  ";
const static PROGMEM prog_uchar JOG4X_MSG[] = "        X-          ";
const static PROGMEM prog_uchar JOG3Y_MSG[] = "  <-X (Ende)  Z->   ";
const static PROGMEM prog_uchar JOG3Z_MSG[] = "  <-Y (Ende)        ";


const static PROGMEM prog_uchar DISTANCESHORT_MSG[] = "SHORT";
const static PROGMEM prog_uchar DISTANCELONG_MSG[] =  "LONG";

#define HEATING_MSG_DEF "Heating:"
#define HEATING_MSG_LEN (sizeof(HEATING_MSG_DEF) - 1)
const static PROGMEM prog_uchar HEATING_MSG[] =        HEATING_MSG_DEF;
// HEATING_SPACES_MSG is a full LCD screen wide (20 spaces)
const static PROGMEM prog_uchar HEATING_SPACES_MSG[] = HEATING_MSG_DEF "            ";

const static PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
const static PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "R Duese:    ---/---C";
const static PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "L Duese:    ---/---C";
const static PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Plattform:  ---/---C";
const static PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Duese:      ---/---C";


const static PROGMEM prog_uchar EXTRUDER_SPACES_MSG[]  = "Duese              ";
const static PROGMEM prog_uchar RIGHT_SPACES_MSG[]     = "Rechtes Tool       ";
const static PROGMEM prog_uchar LEFT_SPACES_MSG[]      = "Linkes Tool        ";
const static PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Plattform          ";
const static PROGMEM prog_uchar RESET1_MSG[]           = "Werkseinstellung";
const static PROGMEM prog_uchar RESET2_MSG[]           = "setzen?";
const static PROGMEM prog_uchar CANCEL_MSG[] 	       = "Druck abbrechen?";
const static PROGMEM prog_uchar CANCEL_FIL_MSG[]       = "Cancel load/unload?";

const static PROGMEM prog_uchar PAUSE_MSG[]      = "Pause  "; // must be same length as the next msg
const static PROGMEM prog_uchar UNPAUSE_MSG[]    = "Unpause"; // must be same length as the prior msg
const static PROGMEM prog_uchar COLD_PAUSE_MSG[] = "Kalte Pause";

const static PROGMEM prog_uchar BUILDING_MSG[]    = "Ich baue gerade     ";
const static PROGMEM prog_uchar CARDCOMMS_MSG[]   = "SD-Karte fehlerhaft." "Verwende eine andere" "SD-Karte.       ";
const static PROGMEM prog_uchar CARDOPENERR_MSG[] = "Datei nicht lesbar  ";
const static PROGMEM prog_uchar CARDNOFILES_MSG[] = "SD-Karte ist leer";
const static PROGMEM prog_uchar NOCARD_MSG[]      = "Keine SD-Karte";
const static PROGMEM prog_uchar CARDERROR_MSG[]   = "SD-Karte Lesefehler";
const static PROGMEM prog_uchar CARDCRC_MSG[]     = "SD-Karte Lesefehler." "Zu viele CRC fehler." "Schlechter Kontakt, " "oder Defekt.";
const static PROGMEM prog_uchar CARDFORMAT_MSG[]  = "Kartenformat nicht  " "lesbar. Formatiere  " "mit FAT-16.";
const static PROGMEM prog_uchar CARDSIZE_MSG[]    = "SD-Karte nicht      " "lesbar, Partition   " "ueber 2GB. Mit      " "FAT-16 Formatieren.";
const static PROGMEM prog_uchar EXTRUDER_TIMEOUT_MSG[] = "Timeout beim heizen " "der Duese."; 
const static PROGMEM prog_uchar PLATFORM_TIMEOUT_MSG[] = "Timeout beim heizen " "der Plattform.";

const static PROGMEM prog_uchar BUILD_MSG[] =            "Drucke von SD";
const static PROGMEM prog_uchar PREHEAT_MSG[] =          "Vorheizen";
const static PROGMEM prog_uchar UTILITIES_MSG[] =        "Utilities";
const static PROGMEM prog_uchar MONITOR_MSG[] =          "Monitor Modus";
const static PROGMEM prog_uchar JOG_MSG[]   =            "Manueller Modus";
const static PROGMEM prog_uchar CALIBRATION_MSG[] =      "Kalibriere Achse";
const static PROGMEM prog_uchar HOME_AXES_MSG[] =        "Achsen zur Startpos.";
const static PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Filament laden";
const static PROGMEM prog_uchar VERSION_MSG[] = 	 "Versions Information";
const static PROGMEM prog_uchar DSTEPS_MSG[] =           "Steppers ausschalten";
const static PROGMEM prog_uchar ESTEPS_MSG[] =           "Steppers einschalten";
const static PROGMEM prog_uchar PLATE_LEVEL_MSG[] =      "Plattform zur Startpos.";
const static PROGMEM prog_uchar PREHEAT_SET_MSG[] =      "Vorheiz Einstellung";
const static PROGMEM prog_uchar SETTINGS_MSG[] =         "Generelle Einstellung";
const static PROGMEM prog_uchar RESET_MSG[] =            "Einstellungen widerherstellen";
const static PROGMEM prog_uchar NOZZLES_MSG[] =          "Duesen kalibrieren";
const static PROGMEM prog_uchar BOT_STATS_MSG[] =        "Bot Statistik";
const static PROGMEM prog_uchar TOOL_COUNT_MSG[] =       "Duesen";
const static PROGMEM prog_uchar SOUND_MSG[] =            "Sound";
const static PROGMEM prog_uchar LED_MSG[] =              "LED Farbe";
const static PROGMEM prog_uchar LED_HEAT_MSG[] =         "Heiz LEDs";
const static PROGMEM prog_uchar EXIT_MSG[] =             "Exit Menue";
const static PROGMEM prog_uchar ACCELERATE_MSG[] = 	 "Beschleunigung";
const static PROGMEM prog_uchar HBP_MSG[]                 = "HBP installiert";
const static PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[] = "Ueberschreibe GcTemp";
const static PROGMEM prog_uchar PAUSE_HEAT_MSG[]	  = "Pause mit Heizung";
const static PROGMEM prog_uchar EXTRUDER_HOLD_MSG[]       = "Duesen halten";
#ifdef TOOLHEAD_OFFSET_SYSTEM
const static PROGMEM prog_uchar NOZZLE_ERROR_MSG[] = "Toolhead Offset Sys " "must be set to NEW  " "to use this utility.";
const static PROGMEM prog_uchar TOOL_OFFSET_SYSTEM_MSG[]  = "Tool Offset Sys";
const static PROGMEM prog_uchar OLD_MSG[]                 = "ALT";
const static PROGMEM prog_uchar NEW_MSG[]                 = "NEU";
#endif
const static PROGMEM prog_uchar SD_USE_CRC_MSG[]          = "Check SD reads";
const static PROGMEM prog_uchar PSTOP_ENABLE_MSG[]        = "P-Stop control";
const static PROGMEM prog_uchar DISABLED_MSG[]            = "N/A";
#ifdef DITTO_PRINT
const static PROGMEM prog_uchar DITTO_PRINT_MSG[]         = "Ditto Printing";
#endif
const static PROGMEM prog_uchar PAUSEATZPOS_MSG[]	  = "Pause bei ZPos";
const static PROGMEM prog_uchar CHANGE_SPEED_MSG[]        = "Aendere Speed";
const static PROGMEM prog_uchar CHANGE_TEMP_MSG[]         = "Aendere Temperatur";
const static PROGMEM prog_uchar FAN_ON_MSG[]              = "Set Cooling Fan ON "; // Needs trailing space
const static PROGMEM prog_uchar FAN_OFF_MSG[]             = "Set Cooling Fan OFF";
const static PROGMEM prog_uchar RETURN_TO_MAIN_MSG[]      = "Hauptmenue";
const static PROGMEM prog_uchar PRINT_ANOTHER_MSG[]       = "Print Another Copy";
const static PROGMEM prog_uchar CANNOT_PRINT_ANOTHER_MSG[]= "(Cannot Print Copy)";


const static PROGMEM prog_uchar RED_COLOR_MSG[]    = "ROT   ";
const static PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE";
const static PROGMEM prog_uchar PINK_COLOR_MSG[]   = "ROSA  ";
const static PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "GRUEN ";
const static PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLAU  ";
const static PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "LILA  ";
const static PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "WEISS ";
const static PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "CUSTOM";
const static PROGMEM prog_uchar OFF_COLOR_MSG[]    = "AUS   ";

const static PROGMEM prog_uchar ERROR_MSG[] =   "fehler!";
const static PROGMEM prog_uchar NA_MSG[] =      "  NA    ";
const static PROGMEM prog_uchar WAITING_MSG[] = "warte...";

const static PROGMEM prog_uchar EXTEMP_CHANGE_MSG[]               = "My temperature was  " "changed externally. " "Reselect filament   " "menu to try again.";
const static PROGMEM prog_uchar HEATER_INACTIVITY_MSG[]           = "Heaters shutdown due" "to inactivity";
const static PROGMEM prog_uchar HEATER_TOOL_MSG[]                 = "Extruder Failure!   ";
const static PROGMEM prog_uchar HEATER_TOOL0_MSG[]                = "Tool 0 Failure!     ";
const static PROGMEM prog_uchar HEATER_TOOL1_MSG[]                = "Tool 1 Failure!     ";
const static PROGMEM prog_uchar HEATER_PLATFORM_MSG[]             = "Platform Failure!   ";

const static PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[] = "Temp limit reached. " "Shutdown or restart.";
const static PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[]     = "Not heating properly" "Check wiring.";
const static PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[]   = "Temperature dropping" "Check wiring.";
const static PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[]  = "Temperature reads   " "are failing.        " "Check wiring.";
const static PROGMEM prog_uchar HEATER_FAIL_READ_MSG[]            = "Temperature reads   " "out of range.       " "Check wiring.";

const static PROGMEM prog_uchar BUILD_TIME_MSG[]	= "Druck Zeit:     h  m"; 
const static PROGMEM prog_uchar Z_POSITION_MSG[]	= "ZPosition:"; 
const static PROGMEM prog_uchar FILAMENT_MSG[]	  = "Filament:"; 
const static PROGMEM prog_uchar PROFILES_MSG[]	  = "Profile"; 
const static PROGMEM prog_uchar HOME_OFFSETS_MSG[]= "Home Offsets"; 
const static PROGMEM prog_uchar MILLIMETERS_MSG[] = "mm";
const static PROGMEM prog_uchar METERS_MSG[]	  = "m";
const static PROGMEM prog_uchar LINE_NUMBER_MSG[] = "Zeile:              ";
const static PROGMEM prog_uchar LEFT_EXIT_MSG []  = "Left Key to Go Back ";

const static PROGMEM prog_uchar BACK_TO_MONITOR_MSG[] ="Back to Monitor";
const static PROGMEM prog_uchar STATS_MSG[]	      = "Druck Statistik";
const static PROGMEM prog_uchar CANCEL_BUILD_MSG[]    = "Cancel Print"; 
const static PROGMEM prog_uchar HEATERS_OFF_MSG[]    = "Heaters Off"; 

const static PROGMEM prog_uchar FILAMENT_ODOMETER_MSG[]    = "Filament Odometer";
const static PROGMEM prog_uchar FILAMENT_LIFETIME1_MSG[]   = "Life:"; 
const static PROGMEM prog_uchar FILAMENT_TRIP1_MSG[]       = "Trip:"; 
const static PROGMEM prog_uchar FILAMENT_LIFETIME2_MSG[]   = "Filament:"; 
const static PROGMEM prog_uchar FILAMENT_TRIP2_MSG[]   	   = "Fil. Trip:"; 
const static PROGMEM prog_uchar FILAMENT_RESET_TRIP_MSG[]  = "  'M' - Reset Trip"; 

const static PROGMEM prog_uchar PROFILE_RESTORE_MSG[]		= "Restore";
const static PROGMEM prog_uchar PROFILE_DISPLAY_CONFIG_MSG[]	= "Display Config";
const static PROGMEM prog_uchar PROFILE_CHANGE_NAME_MSG[]	= "Change Name";
const static PROGMEM prog_uchar PROFILE_SAVE_TO_PROFILE_MSG[]	= "Save To Profile";
const static PROGMEM prog_uchar PROFILE_PROFILE_NAME_MSG[]	= "Profile Name:";
const static PROGMEM prog_uchar UPDNLRM_MSG[]			= "Up/Dn/R/L/M to Set";
const static PROGMEM prog_uchar UPDNLM_MSG[]			= "Up/Dn/Left/M to Set";
const static PROGMEM prog_uchar XYZOFFSET_MSG[]                 = " Offset: ";  // needs extra space
const static PROGMEM prog_uchar PROFILE_RIGHT_MSG[]		= "Right Temp: ";
const static PROGMEM prog_uchar PROFILE_LEFT_MSG[]		= "Left Temp: ";
const static PROGMEM prog_uchar PROFILE_PLATFORM_MSG[]		= "Platform Temp:   ";
const static PROGMEM prog_uchar PAUSE_AT_ZPOS_MSG[]		= "Pause at ZPos: ";

const static PROGMEM prog_uchar PRINTED_TOO_LONG_MSG[]		= "Line:    1000000000+";

const static PROGMEM prog_uchar PAUSE_ENTER_MSG[]		= "Entering Pause...   "; // Needs trailing spaces
const static PROGMEM prog_uchar CANCELLING_ENTER_MSG[]		= "Cancelling Print... "; // Needs trailing spaces
const static PROGMEM prog_uchar PAUSE_LEAVE_MSG[]		= "Leaving Pause...    "; // Needs trailing spaces
const static PROGMEM prog_uchar PAUSE_DRAINING_PIPELINE_MSG[]	= "Draining pipeline..."; // Needs trailing spaces
const static PROGMEM prog_uchar PAUSE_CLEARING_BUILD_MSG[]	= "Clearing build...   "; // Needs trailing spaces
const static PROGMEM prog_uchar PAUSE_RESUMING_POSITION_MSG[]	= "Resuming position..."; // Needs trailing spaces
const static PROGMEM prog_uchar TOTAL_TIME_MSG[]                = "Lifetime:      h 00m";
const static PROGMEM prog_uchar LAST_TIME_MSG[]                 = "Last Print:    h 00m";
const static PROGMEM prog_uchar BUILD_TIME2_MSG[]               =  "Print Time:   h 00m"; // This string is 19 chars WIDE!
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

#endif // __MENU_EN_HH__
