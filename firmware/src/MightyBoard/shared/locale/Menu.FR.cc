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

const PROGMEM prog_uchar GO_MSG[] =         "Prechauffer";
const PROGMEM prog_uchar STOP_MSG[] =       "Refroidir";
const PROGMEM prog_uchar PLATFORM_MSG[] =   "Platforme";
const PROGMEM prog_uchar TOOL_MSG[] =       "Extrudeur";
const PROGMEM prog_uchar START_TEST_MSG[]=  "I'm going to print  " "a series of lines so" "we can find my      " "nozzle alignment.   ";
const PROGMEM prog_uchar EXPLAIN1_MSG[] =   "Look for the best   " "matched line in each" "axis set.  Lines are" "numbered 1-13 and...";
const PROGMEM prog_uchar EXPLAIN2_MSG[] =   "line one is extra   " "long. The Y axis set" "is left on the plate" "and X axis is right.";
const PROGMEM prog_uchar END_MSG  [] =      "Great!  I've saved  " "these settings and  " "I'll use them to    " "make nice prints!   ";

const PROGMEM prog_uchar SELECT_MSG[] =     "Meilleure ligne     ";
const PROGMEM prog_uchar DONE_MSG[]   =     "Fini";

const PROGMEM prog_uchar NO_MSG[]   =       "Non";
const PROGMEM prog_uchar YES_MSG[]  =       "Oui";
const uint8_t YES_NO_WIDTH = 3;

const PROGMEM prog_uchar XAXIS_MSG[] =      "Axe X";
const PROGMEM prog_uchar YAXIS_MSG[] =      "Axe Y";
const PROGMEM prog_uchar HEATER_ERROR_MSG[]=  "Les extrudeurs ne   " "chauffent pas.      " "Verifiez les        " "connexions!";
const PROGMEM prog_uchar STOP_EXIT_MSG[]   = "Quand le filament   " "sort de la buse,    " "appuyez sur M pour  " "quitter.";
const PROGMEM prog_uchar TIMEOUT_MSG[]  	  = "5 minute timeout has" "elapsed.  Press 'M' " "to exit.";

const PROGMEM prog_uchar LOAD_RIGHT_MSG[]    = "Mettre a droite";
const PROGMEM prog_uchar LOAD_LEFT_MSG[]     = "Mettre a gauche";
const PROGMEM prog_uchar LOAD_SINGLE_MSG[]   = "Mettre le plastique";
const PROGMEM prog_uchar UNLOAD_SINGLE_MSG[] = "Retirer le plastique";
const PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]  = "Retirer a droite";
const PROGMEM prog_uchar UNLOAD_LEFT_MSG[]   = "Retirer a gauche";

const PROGMEM prog_uchar JOG1_MSG[]  = "    Mode manuel     ";
const PROGMEM prog_uchar JOG2X_MSG[] = "        X+          ";
const PROGMEM prog_uchar JOG3X_MSG[] = "      Retour  Y->   ";
const PROGMEM prog_uchar JOG4X_MSG[] = "        X-          ";
const PROGMEM prog_uchar JOG3Y_MSG[] = "  <-X Retour  Z->   ";
const PROGMEM prog_uchar JOG3Z_MSG[] = "  <-Y Retour        ";


const PROGMEM prog_uchar DISTANCESHORT_MSG[] = "COURT";
const PROGMEM prog_uchar DISTANCELONG_MSG[] =  "LONG";

#define HEATING_MSG_DEF "Chauffe:"
const PROGMEM prog_uchar HEATING_MSG[] =        HEATING_MSG_DEF;
// HEATING_SPACES_MSG is a full LCD screen wide (20 spaces)
const PROGMEM prog_uchar HEATING_SPACES_MSG[] = HEATING_MSG_DEF "            ";

const PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
const PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "Tete Droite ---/---C";
const PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "Tete Gauche ---/---C";
const PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Plateforme  ---/---C";
const PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Extrudeur:  ---/---C";
const PROGMEM prog_uchar FILAMENT_CANCEL_MSG[] =  "Press left to cancel";

const PROGMEM prog_uchar EXTRUDER_SPACES_MSG[]  = "Extruder           ";
const PROGMEM prog_uchar RIGHT_SPACES_MSG[]     = "Tete de droite     ";
const PROGMEM prog_uchar LEFT_SPACES_MSG[]      = "Tete de gauche     ";
const PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Platforme          ";
const PROGMEM prog_uchar RESET1_MSG[]           = "Retour parametres";
const PROGMEM prog_uchar RESET2_MSG[]           = "d'usine?";
const PROGMEM prog_uchar CANCEL_MSG[] = "Annuler impression?";
const PROGMEM prog_uchar CANCEL_FIL_MSG[] = "Cancel load/unload?";

const PROGMEM prog_uchar PAUSE_MSG[]      = "Pause    "; // must be same length as the next msg
const PROGMEM prog_uchar UNPAUSE_MSG[]    = "Reprendre"; // must be same length as the previous msg
const PROGMEM prog_uchar COLD_PAUSE_MSG[] = "Cold Pause";

const PROGMEM prog_uchar BUILDING_MSG[]    = "Pret a imprimer     ";
const PROGMEM prog_uchar CARDCOMMS_MSG[]   = "J'ai du mal a lire  " "la carte SD. Essayez" "une autre carte.";
const PROGMEM prog_uchar CARDOPENERR_MSG[] = "Cannot open the file";
const PROGMEM prog_uchar CARDNOFILES_MSG[] = "Carte SD vide   ";
const PROGMEM prog_uchar NOCARD_MSG[]      = "Pas de carte SD    ";
const PROGMEM prog_uchar CARDERROR_MSG[]   = "Erreur de lecture   " "sur la carte SD.    ";
const PROGMEM prog_uchar CARDCRC_MSG[]     = "SD card read error. " "Too many CRC errors." "Bad card contacts or" "electrical noise.";
const PROGMEM prog_uchar CARDFORMAT_MSG[]  = "Systeme de fichier  " "de la carte SD est  " "inconnu. Formatez en" "FAT-16 ou en FAT-32.";
const PROGMEM prog_uchar CARDSIZE_MSG[]    = "Partition de la     " "carte SD supperieur " "a 2GB. formatez en  " "FAT-16 ou en FAT-32.";
const PROGMEM prog_uchar EXTRUDER_TIMEOUT_MSG[] = "Delai dépassé       pour chauffer       l'extruder.";
const PROGMEM prog_uchar PLATFORM_TIMEOUT_MSG[] = "Délai dépassé       pour chauffer       la plateforme.";

const PROGMEM prog_uchar BUILD_MSG[] =            "Imprimer depuis SD";
const PROGMEM prog_uchar PREHEAT_MSG[] =          "Prechauffage";
const PROGMEM prog_uchar UTILITIES_MSG[] =        "Utilitaires";
const PROGMEM prog_uchar MONITOR_MSG[] =          "Visu Temp   ";
const PROGMEM prog_uchar JOG_MSG[]   =            "Mode Manuel";
const PROGMEM prog_uchar CALIBRATION_MSG[] =      "Calibration des axes";
const PROGMEM prog_uchar HOME_AXES_MSG[] =        "Axes a l'origine";
const PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Options du filament";
const PROGMEM prog_uchar VERSION_MSG[] = 		     "Numero de Version";
const PROGMEM prog_uchar DSTEPS_MSG[] =           "Debloquer Moteurs";
const PROGMEM prog_uchar ESTEPS_MSG[] =           "Bloquer Moteurs  ";
const PROGMEM prog_uchar PLATE_LEVEL_MSG[] =      "Relever Plateforme";
const PROGMEM prog_uchar PREHEAT_SET_MSG[] =      "Param. prechauffage ";
const PROGMEM prog_uchar SETTINGS_MSG[] =         "Parametres Generaux";
const PROGMEM prog_uchar RESET_MSG[] =            "RAZ usine";
const PROGMEM prog_uchar NOZZLES_MSG[] =          "Calibration Buses";
const PROGMEM prog_uchar BOT_STATS_MSG[] =        "Bot Statistics";
const PROGMEM prog_uchar TOOL_COUNT_MSG[] =       "Nb Tetes:";
const PROGMEM prog_uchar SOUND_MSG[] =            "Son";
//const PROGMEM prog_uchar LED_HEAT_MSG[] =         "Coul Chauf";
const PROGMEM prog_uchar EXIT_MSG[] =             "Sortir du Menu";
const PROGMEM prog_uchar ACCELERATE_MSG[] = 	     "Acceleration";
const PROGMEM prog_uchar HBP_MSG[]                 = "HBP installe";
const PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[] = "Surcharger GcTemp";
const PROGMEM prog_uchar PAUSE_HEAT_MSG[]	        = "Pause avec chauffe";
const PROGMEM prog_uchar EXTRUDER_HOLD_MSG[]       = "Extruder Hold";
const PROGMEM prog_uchar SD_USE_CRC_MSG[]          = "Check SD reads";
#if defined(PSTOP_SUPPORT)
#if defined(ZYYX_3D_PRINTER)
const PROGMEM prog_uchar PSTOP_ENABLE_MSG[]        = "Capteur filament";
const PROGMEM prog_uchar PSTOP_INVERTED_MSG[]      = "Capteurs inverses";
const PROGMEM prog_uchar PSTOP_MSG[]               = "Probleme filament   " "detecte. Verifiez la" "bobine du filament. " "GAUCHE pour le menu.";
#else
const PROGMEM prog_uchar PSTOP_ENABLE_MSG[]        = "P-Stop Control";
const PROGMEM prog_uchar PSTOP_INVERTED_MSG[]      = "P-Stop Inverse";
const PROGMEM prog_uchar PSTOP_MSG[]               = "P-Stop declanche";
#endif
#endif
const PROGMEM prog_uchar DISABLED_MSG[]            = "N/A";
//#ifdef DITTO_PRINT
const PROGMEM prog_uchar DITTO_PRINT_MSG[]         = "Impression identique";
//#endif
const PROGMEM prog_uchar PAUSEATZPOS_MSG[]	        = "Pause a ZPos";
const PROGMEM prog_uchar CHANGE_SPEED_MSG[]        = "Changer la vitesse";
const PROGMEM prog_uchar CHANGE_TEMP_MSG[]         = "Changer Temperature";
const PROGMEM prog_uchar CHANGE_HBP_TEMP_MSG[]     = "Changer Temp. HBP";
const PROGMEM prog_uchar FAN_ON_MSG[]              = "Activer ventilateur"; // Needs trailing space
const PROGMEM prog_uchar FAN_OFF_MSG[]             = "Desactiver ventilo.";
const PROGMEM prog_uchar RETURN_TO_MAIN_MSG[]      = "Menu principal";
const PROGMEM prog_uchar PRINT_ANOTHER_MSG[]       = "Imprimer une copie";
const PROGMEM prog_uchar CANNOT_PRINT_ANOTHER_MSG[]= "(Copie impossible)";

#if defined(HAS_RGB_LED)
const PROGMEM prog_uchar LIGHTS_OFF_MSG[]   = "Desactiver eclerage";
const PROGMEM prog_uchar LIGHTS_ON_MSG[]    = "Activer eclerage";
#if defined(RGB_LED_MENU)
const PROGMEM prog_uchar LED_MSG[]          = "Eclairage";
const PROGMEM prog_uchar RED_COLOR_MSG[]    = "ROUGE ";
const PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE";
const PROGMEM prog_uchar PINK_COLOR_MSG[]   = "ROSE  ";
const PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "VERT  ";
const PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLEU  ";
const PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "VIOLET";
const PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "BLANC ";
const PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "AUTRE ";
const PROGMEM prog_uchar OFF_COLOR_MSG[]    = "OFF   ";
#endif
#endif

const PROGMEM prog_uchar ERROR_MSG[] =   "erreur";
const PROGMEM prog_uchar NA_MSG[] =      "  NA    ";
const PROGMEM prog_uchar WAITING_MSG[] = "attente ";

const PROGMEM prog_uchar FILCANCEL_MSG[]                   = "Charge/Decharge du  " "filament annule du a" "une erreur chauffe";
const PROGMEM prog_uchar EXTEMP_CHANGE_MSG[]               = "Changement externe  " "de la temperature.  " "Reselectionnez le   " "filament via menu.";
const PROGMEM prog_uchar HEATER_INACTIVITY_MSG[]           = "Exctincion des tetes" "due a l'inactivite";
const PROGMEM prog_uchar HEATER_TOOL_MSG[]                 = "Echec extruder!     ";
const PROGMEM prog_uchar HEATER_TOOL0_MSG[]                = "Tool 0 Failure!     ";
const PROGMEM prog_uchar HEATER_TOOL1_MSG[]                = "Tool 1 Failure!     ";
const PROGMEM prog_uchar HEATER_PLATFORM_MSG[]             = "Echec plateforme!   ";

//The following messages are in english because the french translation is too large
//and results in a firmware image that does not fit in the available flash memory space
//Please try to save bytes maybe by using some abreviations
const PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[] = "Temp limit reached. " "Shutdown or restart.";
const PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[]     = "Not heating properly" "Check wiring.";
const PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[]   = "Temperature dropping" "Check wiring.";
const PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[]  = "Temperature reads   " "are failing.        " "Check wiring.";
//const PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[] = "Surchauffe des tetes" "Temperature limite  " "Atteinte! Veuillez  " "Redemarrer";
//const PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[]     = "Surchauffe des tetes" "Elles ne chauffent  " "pas correctement !  " "Verif. connectiques ";
//const PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[]   = "Echec du chauffage !" "La temperature des  " "tetes chute !       " "Verif. connectiques ";
//const PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[]  = "Erreur de chauffe ! " "Echec du releve de  " "temperature!        " "Verif. connectiques ";
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
const PROGMEM prog_uchar LEFT_EXIT_MSG []  = "Gauche pour retour";

const PROGMEM prog_uchar BACK_TO_MONITOR_MSG[] ="Retour moniteur";
const PROGMEM prog_uchar STATS_MSG[]			= "Stat. impression";
const PROGMEM prog_uchar CANCEL_BUILD_MSG[]    = "Annuler impress.";
const PROGMEM prog_uchar HEATERS_OFF_MSG[]    = "Heaters Off";

const PROGMEM prog_uchar FILAMENT_ODOMETER_MSG[]    = "Filament Odometer";
const PROGMEM prog_uchar FILAMENT_LIFETIME1_MSG[]   = "Life:";
const PROGMEM prog_uchar FILAMENT_TRIP1_MSG[]       = "Trip:";
const PROGMEM prog_uchar FILAMENT_LIFETIME2_MSG[]   = "Filament:";
const PROGMEM prog_uchar FILAMENT_TRIP2_MSG[]   	   = "Fil. Trip:";
const PROGMEM prog_uchar FILAMENT_RESET_TRIP_MSG[]  = "  'M' - Reset Trip";

const PROGMEM prog_uchar PROFILE_RESTORE_MSG[]		= "Restore";
const PROGMEM prog_uchar PROFILE_DISPLAY_CONFIG_MSG[]	= "Display Config";
const PROGMEM prog_uchar PROFILE_CHANGE_NAME_MSG[]	= "Change Name";
const PROGMEM prog_uchar PROFILE_SAVE_TO_PROFILE_MSG[]	= "Save To Profile";
const PROGMEM prog_uchar PROFILE_PROFILE_NAME_MSG[]	= "Profile Name:";
const PROGMEM prog_uchar UPDNLRM_MSG[]			= "Up/Dn/R/L/M to set";
const PROGMEM prog_uchar UPDNLM_MSG[]			= "Up/Dn/Left/M to set";
const PROGMEM prog_uchar XYZOFFSET_MSG[]                 = " Offset:";  // needs extra space
const PROGMEM prog_uchar XYZTOOLHEAD_MSG[]              = " Toolhead Offset:"; // needs extra space
const PROGMEM prog_uchar TOOLHEAD_OFFSETS_MSG[]         = "Toolhead Offsets";
const PROGMEM prog_uchar PROFILE_RIGHT_MSG[]		= "Temp. droite: ";
const PROGMEM prog_uchar PROFILE_LEFT_MSG[]		= "Temp gauche: ";
const PROGMEM prog_uchar PROFILE_PLATFORM_MSG[]		= "Platforme Temp:   ";
const PROGMEM prog_uchar PAUSE_AT_ZPOS_MSG[]		= "Pause a ZPos: ";

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

#ifdef ALTERNATE_UART
const PROGMEM prog_uchar ALT_UART_MSG[] = "Serial I/O";
const PROGMEM prog_uchar ALT_UART_0_MSG[] = "  USB";
const PROGMEM prog_uchar ALT_UART_1_MSG[] = "UART1";
#endif

#if defined(AUTO_LEVEL)
const PROGMEM prog_uchar ALEVEL_UTILITY_COMP_MSG[] = "Auto-level Adjust";
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
