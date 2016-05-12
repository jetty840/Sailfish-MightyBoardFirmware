#include "Model.hh"
#include "locale.hh"

const PROGMEM prog_uchar ON_MSG[]            = "EIN";
const PROGMEM prog_uchar OFF_MSG[]           = "AUS";

//#ifdef STACK_PAINT
const PROGMEM prog_uchar SPLASH_SRAM_MSG[]   = "SRAM frei";
//#endif

const PROGMEM prog_uchar SPLASH1A_MSG[]      = "    FAIL!           ";
const PROGMEM prog_uchar SPLASH2A_MSG[]      = "    SUCCESS!        ";
const PROGMEM prog_uchar SPLASH3A_MSG[]      = "connected correctly ";
const PROGMEM prog_uchar SPLASH4A_MSG[]      = "Heaters are not     ";

const PROGMEM prog_uchar GO_MSG[]            = "Starte vorheizen";
const PROGMEM prog_uchar STOP_MSG[]          = "Schalte Heiz.aus";
const PROGMEM prog_uchar PLATFORM_MSG[]      = "Plattform";
const PROGMEM prog_uchar TOOL_MSG[]          = "Duese";
const PROGMEM prog_uchar START_TEST_MSG[]    = "Ich drucke eine     "
                                               "Serie von Linien,   "
                                               "um die Duesen       "
                                               "auszurichten.       ";
const PROGMEM prog_uchar EXPLAIN1_MSG[]      = "Finde die besten    "
                                               "uebereinstimmenden  "
                                               "Linien je Achse.    "
                                               "Linien sind von 1-13";
const PROGMEM prog_uchar EXPLAIN2_MSG[]      = "Linie eins          "
                                               "ist extra lang.     "
                                               "Y Achse ist links   "
                                               "X Achse ist rechts. ";
const PROGMEM prog_uchar END_MSG  []         = "   Einstellungen    "
                                               "       wurden       "
                                               "    erfolgreich     "
                                               "    gespeichert!    ";

const PROGMEM prog_uchar SELECT_MSG[]        = "Select best lines:";
const PROGMEM prog_uchar DONE_MSG[]          = "Fertig";

const PROGMEM prog_uchar NO_MSG[]            = "NEIN";  // length 4 
const PROGMEM prog_uchar YES_MSG[]           = "  JA";  // length 4 
const uint8_t YES_NO_WIDTH = 4;

const PROGMEM prog_uchar XAXIS_MSG[]         = "X Achse    ";
const PROGMEM prog_uchar YAXIS_MSG[]         = "Y Achse    ";

const PROGMEM prog_uchar HEATER_ERROR_MSG[]  = "Duesen heizen nicht "
                                               "Pruefe die Kabel.   "
                                               " ";

const PROGMEM prog_uchar STOP_EXIT_MSG[]     = "Beginne Laden oder  "
                                               "Entladen. Druecke'M'"
                                               "zum beenden.";
const PROGMEM prog_uchar TIMEOUT_MSG[]       = "5 Minuten timeout   "
                                               "wurden erreicht 'M' "
                                               "zum beenden.";

const PROGMEM prog_uchar LOAD_RIGHT_MSG[]    = "Lade rechts";
const PROGMEM prog_uchar LOAD_LEFT_MSG[]     = "Lade links";
const PROGMEM prog_uchar LOAD_SINGLE_MSG[]   = "Lade";
const PROGMEM prog_uchar UNLOAD_SINGLE_MSG[] = "Entlade";
const PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]  = "Entlade rechts";
const PROGMEM prog_uchar UNLOAD_LEFT_MSG[]   = "Entlade links";

const PROGMEM prog_uchar JOG1_MSG[]  = "      Manuell       ";
const PROGMEM prog_uchar JOG2X_MSG[] = "        X+          ";
const PROGMEM prog_uchar JOG3X_MSG[] = "      (Ende)  Y->   ";
const PROGMEM prog_uchar JOG4X_MSG[] = "        X-          ";
const PROGMEM prog_uchar JOG3Y_MSG[] = " <-X  (Ende)  Z->   ";
const PROGMEM prog_uchar JOG3Z_MSG[] = " <-Y  (Ende)        ";


const PROGMEM prog_uchar DISTANCESHORT_MSG[] = "SHORT";
const PROGMEM prog_uchar DISTANCELONG_MSG[] =  "LONG";

#define HEATING_MSG_DEF "Heizen:"
const PROGMEM prog_uchar HEATING_MSG[] =        HEATING_MSG_DEF;
// HEATING_SPACES_MSG is a full LCD screen wide (20 spaces)
const PROGMEM prog_uchar HEATING_SPACES_MSG[] = HEATING_MSG_DEF "             ";

const PROGMEM prog_uchar BUILD_PERCENT_MSG[]    = " --%";
const PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[]   = "R Duese:    ---/---C";
const PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[]   = "L Duese:    ---/---C";
const PROGMEM prog_uchar PLATFORM_TEMP_MSG[]    = "Plattform:  ---/---C";
const PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]    = "Duese:      ---/---C";
const PROGMEM prog_uchar FILAMENT_CANCEL_MSG[]  = "Press left to cancel";


const PROGMEM prog_uchar EXTRUDER_SPACES_MSG[]  = "Duese              ";
const PROGMEM prog_uchar RIGHT_SPACES_MSG[]     = "Rechtes Tool       ";
const PROGMEM prog_uchar LEFT_SPACES_MSG[]      = "Linkes Tool        ";
const PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Plattform          ";
const PROGMEM prog_uchar RESET1_MSG[]           = "Werkseinstellung";
const PROGMEM prog_uchar RESET2_MSG[]           = "setzen?";
const PROGMEM prog_uchar CANCEL_MSG[]           = "Druck abbrechen?";
const PROGMEM prog_uchar CANCEL_FIL_MSG[]       = "Cancel load/unload?";

const PROGMEM prog_uchar PAUSE_MSG[]      = "Pause "; // must be same length as the next msg
const PROGMEM prog_uchar UNPAUSE_MSG[]    = "Weiter"; // must be same length as the prior msg
const PROGMEM prog_uchar COLD_PAUSE_MSG[] = "Kalte Pause";

const PROGMEM prog_uchar BUILDING_MSG[]            = "Ich baue gerade     ";
const PROGMEM prog_uchar CARDCOMMS_MSG[]           = "SD-Karte fehlerhaft."
                                                     "Verwende eine andere"
                                                     "SD-Karte.       ";
const PROGMEM prog_uchar CARDOPENERR_MSG[]         = "Datei nicht lesbar  ";
const PROGMEM prog_uchar CARDNOFILES_MSG[]         = "SD-Karte ist leer";
const PROGMEM prog_uchar NOCARD_MSG[]              = "Keine SD-Karte";
const PROGMEM prog_uchar CARDERROR_MSG[]           = "SD-Karte Lesefehler";
const PROGMEM prog_uchar CARDCRC_MSG[]             = "SD-Karte Lesefehler."
                                                     "Zu viele CRC fehler."
                                                     "Schlechter Kontakt, "
                                                     "oder Defekt.";
const PROGMEM prog_uchar CARDFORMAT_MSG[]          = "Kartenformat nicht  "
                                                     "lesbar. Formatiere  "
                                                     "mit FAT-16 oder     "
                                                     "FAT-32.";
const PROGMEM prog_uchar CARDSIZE_MSG[]            = "SD-Karte nicht      "
                                                     "lesbar, Partition   "
                                                     "ueber 2GB. Mit      "
                                                     "FAT-16 Formatieren.";
const PROGMEM prog_uchar EXTRUDER_TIMEOUT_MSG[]    = "Timeout beim heizen "
                                                     "der Duese.";
const PROGMEM prog_uchar PLATFORM_TIMEOUT_MSG[]    = "Timeout beim heizen "
                                                     "der Plattform.";

const PROGMEM prog_uchar BUILD_MSG[]               = "Drucke von SD";
const PROGMEM prog_uchar PREHEAT_MSG[]             = "Vorheizen";
const PROGMEM prog_uchar UTILITIES_MSG[]           = "Utilities";
const PROGMEM prog_uchar MONITOR_MSG[]             = "Monitor Modus";
const PROGMEM prog_uchar JOG_MSG[]                 = "Manueller Modus";
const PROGMEM prog_uchar CALIBRATION_MSG[]         = "Kalibriere Achse";
const PROGMEM prog_uchar HOME_AXES_MSG[]           = "Achsen zur Startpos";
const PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[]    = "Filament laden";
const PROGMEM prog_uchar VERSION_MSG[]             = "Versions Info      ";
const PROGMEM prog_uchar DSTEPS_MSG[]              = "Steppers ausschalt.";
const PROGMEM prog_uchar ESTEPS_MSG[]              = "Steppers einschalt.";
const PROGMEM prog_uchar PLATE_LEVEL_MSG[]         = "Plattform heben";
const PROGMEM prog_uchar PREHEAT_SET_MSG[]         = "Vorheiz Einstellung";
const PROGMEM prog_uchar SETTINGS_MSG[]            = "Generelle Einst.   ";
const PROGMEM prog_uchar RESET_MSG[]               = "Einstellungen RESET";
const PROGMEM prog_uchar NOZZLES_MSG[]             = "Duesen kalibrieren";
const PROGMEM prog_uchar BOT_STATS_MSG[]           = "Bot Statistik";
const PROGMEM prog_uchar TOOL_COUNT_MSG[]          = "Duesen";
const PROGMEM prog_uchar SOUND_MSG[]               = "Sound";
//const PROGMEM prog_uchar LED_HEAT_MSG[]            = "Heiz LEDs";
const PROGMEM prog_uchar EXIT_MSG[]                = "Exit Menue";
const PROGMEM prog_uchar ACCELERATE_MSG[]          = "Beschleunigung";
const PROGMEM prog_uchar HBP_MSG[]                 = "HBP installiert";
const PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[] = "Override GcTemp "; /* max lenght 16 */
const PROGMEM prog_uchar PAUSE_HEAT_MSG[]          = "Pause mit Heiz. ";
const PROGMEM prog_uchar EXTRUDER_HOLD_MSG[]       = "Duesen halten";
//#ifdef TOOLHEAD_OFFSET_SYSTEM
const PROGMEM prog_uchar NOZZLE_ERROR_MSG[]        = "Toolhead Offset Sys "
                                                     "must be set to NEW  "
                                                     "to use this utility.";
const PROGMEM prog_uchar TOOL_OFFSET_SYSTEM_MSG[]  = "Tool Offset Sys";
const PROGMEM prog_uchar OLD_MSG[]                 = "ALT";
const PROGMEM prog_uchar NEW_MSG[]                 = "NEU";
//#endif
const PROGMEM prog_uchar SD_USE_CRC_MSG[]          = "Check SD reads";
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
const PROGMEM prog_uchar DITTO_PRINT_MSG[]         = "Doppeldruck";
//#endif
const PROGMEM prog_uchar PAUSEATZPOS_MSG[]         = "Pause bei ZPos";
const PROGMEM prog_uchar CHANGE_SPEED_MSG[]        = "Aendere Speed";
const PROGMEM prog_uchar CHANGE_TEMP_MSG[]         = "Aendere Temperatur";
const PROGMEM prog_uchar CHANGE_HBP_TEMP_MSG[]     = "Aendere HBP Temp";
const PROGMEM prog_uchar FAN_ON_MSG[]              = "Set Cooling Fan ON "; // Needs trailing space
const PROGMEM prog_uchar FAN_OFF_MSG[]             = "Set Cooling Fan OFF";
const PROGMEM prog_uchar RETURN_TO_MAIN_MSG[]      = "Hauptmenue";
const PROGMEM prog_uchar PRINT_ANOTHER_MSG[]       = "Print Another Copy";
const PROGMEM prog_uchar CANNOT_PRINT_ANOTHER_MSG[]= "(Cannot Print Copy)";

#if defined(HAS_RGB_LED)
const PROGMEM prog_uchar LIGHTS_OFF_MSG[]   = "Set Lights OFF";
const PROGMEM prog_uchar LIGHTS_ON_MSG[]    = "Set Lights ON ";
#if defined(RGB_LED_MENU)
const PROGMEM prog_uchar LED_MSG[]          = "LED Farbe";
const PROGMEM prog_uchar RED_COLOR_MSG[]    = "ROT   ";
const PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE";
const PROGMEM prog_uchar PINK_COLOR_MSG[]   = "ROSA  ";
const PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "GRUEN ";
const PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLAU  ";
const PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "LILA  ";
const PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "WEISS ";
const PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "CUSTOM";
const PROGMEM prog_uchar OFF_COLOR_MSG[]    = "AUS   ";
#endif
#endif

const PROGMEM prog_uchar ERROR_MSG[]   = "fehler!";
const PROGMEM prog_uchar NA_MSG[]      = "  NA    ";
const PROGMEM prog_uchar WAITING_MSG[] = "warte...";

const PROGMEM prog_uchar FILCANCEL_MSG[]                      = "Filament load/unload" "canceled due to the " "prior heater error";
const PROGMEM prog_uchar EXTEMP_CHANGE_MSG[]                  = "My temperature was  "
                                                                "changed externally. "
                                                                "Reselect filament   "
                                                                "menu to try again.";
const PROGMEM prog_uchar HEATER_INACTIVITY_MSG[]              = "Heaters shutdown due"
                                                                "to inactivity";
const PROGMEM prog_uchar HEATER_TOOL_MSG[]                    = "Extruder Failure!   ";
const PROGMEM prog_uchar HEATER_TOOL0_MSG[]                   = "Tool 0 Failure!     ";
const PROGMEM prog_uchar HEATER_TOOL1_MSG[]                   = "Tool 1 Failure!     ";
const PROGMEM prog_uchar HEATER_PLATFORM_MSG[]                = "Platform Failure!   ";

const PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[]    = "Temp limit reached. "
                                                                "Shutdown or restart.";
const PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[]        = "Not heating properly"
                                                                "Check wiring.";
const PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[]      = "Temperature dropping"
                                                                "Check wiring.";
const PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[]     = "Temperature reads   "
                                                                "are failing.        "
                                                                "Check wiring.";
const PROGMEM prog_uchar HEATER_FAIL_READ_MSG[]               = "Temperature reads   "
                                                                "out of range.       "
                                                                "Check wiring.";

const PROGMEM prog_uchar BUILD_TIME_MSG[]    = "Druck Zeit:     h  m";

// Spaces needed at end to deal with alternation with auto-level status
const PROGMEM prog_uchar Z_POSITION_MSG[]    = "ZPosition:   ";

const PROGMEM prog_uchar FILAMENT_MSG[]      = "Filament:";
const PROGMEM prog_uchar MON_FILAMENT_MSG[]  = "Filament:      0.0mm";
const PROGMEM prog_uchar PROFILES_MSG[]      = "Profil";
const PROGMEM prog_uchar HOME_OFFSETS_MSG[]  = "Home Offsets";
const PROGMEM prog_uchar MILLIMETERS_MSG[]   = "mm";
const PROGMEM prog_uchar METERS_MSG[]        = "m";
const PROGMEM prog_uchar LINE_NUMBER_MSG[]   = "Linie:              ";
const PROGMEM prog_uchar LEFT_EXIT_MSG []    = "Linke Taste beendet ";

const PROGMEM prog_uchar BACK_TO_MONITOR_MSG[] = "Back to Monitor";
const PROGMEM prog_uchar STATS_MSG[]           = "Druck Statistik";
const PROGMEM prog_uchar CANCEL_BUILD_MSG[]    = "Cancel Print";
const PROGMEM prog_uchar HEATERS_OFF_MSG[]     = "Heaters Off";

const PROGMEM prog_uchar FILAMENT_ODOMETER_MSG[]    = "Filament Odometer";
const PROGMEM prog_uchar FILAMENT_LIFETIME1_MSG[]   = "Life:";
const PROGMEM prog_uchar FILAMENT_TRIP1_MSG[]       = "Trip:";
const PROGMEM prog_uchar FILAMENT_LIFETIME2_MSG[]   = "Filament:";
const PROGMEM prog_uchar FILAMENT_TRIP2_MSG[]       = "Fil. Trip:";
const PROGMEM prog_uchar FILAMENT_RESET_TRIP_MSG[]  = "  'M' - Reset Trip";

const PROGMEM prog_uchar PROFILE_RESTORE_MSG[]           = "Restore";
const PROGMEM prog_uchar PROFILE_DISPLAY_CONFIG_MSG[]    = "Display Config";
const PROGMEM prog_uchar PROFILE_CHANGE_NAME_MSG[]       = "Aendere Name";
const PROGMEM prog_uchar PROFILE_SAVE_TO_PROFILE_MSG[]   = "Speichere Profil";
const PROGMEM prog_uchar PROFILE_PROFILE_NAME_MSG[]      = "Profil Name:";
const PROGMEM prog_uchar UPDNLRM_MSG[]                   = "Up/Dn/R/L/M to set";
const PROGMEM prog_uchar UPDNLM_MSG[]                    = "Up/Dn/Left/M to set";
const PROGMEM prog_uchar XYZOFFSET_MSG[]                 = " Offset:";  // needs extra space
const PROGMEM prog_uchar XYZTOOLHEAD_MSG[]              = " Toolhead Offset:"; // needs extra space
const PROGMEM prog_uchar TOOLHEAD_OFFSETS_MSG[]         = "Toolhead Offsets";
const PROGMEM prog_uchar PROFILE_RIGHT_MSG[]             = "Right Temp: ";
const PROGMEM prog_uchar PROFILE_LEFT_MSG[]              = "Left Temp: ";
const PROGMEM prog_uchar PROFILE_PLATFORM_MSG[]          = "Platform Temp:   ";
const PROGMEM prog_uchar PAUSE_AT_ZPOS_MSG[]             = "Pause bei ZPos: ";

const PROGMEM prog_uchar PRINTED_TOO_LONG_MSG[]          = "Line:    1000000000+";

const PROGMEM prog_uchar PAUSE_ENTER_MSG[]               = "Entering Pause...   "; // Needs trailing spaces
const PROGMEM prog_uchar CANCELLING_ENTER_MSG[]          = "Cancelling print... "; // Needs trailing spaces
const PROGMEM prog_uchar PAUSE_LEAVE_MSG[]               = "Leaving pause...    "; // Needs trailing spaces
const PROGMEM prog_uchar PAUSE_DRAINING_PIPELINE_MSG[]   = "Draining pipeline..."; // Needs trailing spaces
const PROGMEM prog_uchar PAUSE_CLEARING_BUILD_MSG[]      = "Clearing build...   "; // Needs trailing spaces
const PROGMEM prog_uchar PAUSE_RESUMING_POSITION_MSG[]   = "Resuming position..."; // Needs trailing spaces
const PROGMEM prog_uchar TOTAL_TIME_MSG[]                = "Lifetime:      h 00m";
const PROGMEM prog_uchar LAST_TIME_MSG[]                 = "Last Print:    h 00m";
const PROGMEM prog_uchar BUILD_TIME2_MSG[]               =  "Print Time:   h 00m"; // This string is 19 chars WIDE!

#if defined(EEPROM_MENU_ENABLE)
const PROGMEM prog_uchar EEPROM_MSG[]           = "Eeprom";
const PROGMEM prog_uchar EEPROM_DUMP_MSG[]      = "Eeprom -> SD";
const PROGMEM prog_uchar EEPROM_RESTORE_MSG[]   = "SD -> Eeprom";
const PROGMEM prog_uchar EEPROM_ERASE_MSG[]     = "Erase Eeprom";
#endif

#ifdef ALTERNATE_UART
const PROGMEM prog_uchar ALT_UART_MSG[] = "Serial I/O";
const PROGMEM prog_uchar ALT_UART_0_MSG[] = "  USB";
const PROGMEM prog_uchar ALT_UART_1_MSG[] = "UART1";
#endif

#if defined(AUTO_LEVEL)
const PROGMEM prog_uchar ALEVEL_UTILITY_COMP_MSG[] = "Auto-level Adj";
const PROGMEM prog_uchar ALEVEL_COMP_OFFSET_MSG[]  = " Deflection"; // needs extra space
const PROGMEM prog_uchar ALEVEL_UTILITY_MSG[]    = "Auto-level Variance";
const PROGMEM prog_uchar ALEVEL_SCREEN_MSG1[]    = "Max height variance";
const PROGMEM prog_uchar ALEVEL_SCREEN_MSG2[]    = "between probe pts.";
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
