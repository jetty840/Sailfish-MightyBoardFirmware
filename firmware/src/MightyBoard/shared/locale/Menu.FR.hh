#ifndef __MENU_FR_HH__

#define __MENU_FR_HH__

#ifndef FRENCH
#	error no french local defined!
#endif

#ifdef MODEL_REPLICATOR
#if defined(FF_CREATOR)
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish FF Creator ";
#else
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish Replicator1";
#endif
#if !defined(HEATERS_ON_STEROIDS) || defined(FF_CREATOR)
const static PROGMEM prog_uchar SPLASH2_MSG[] = "--------------------";
#else
const static PROGMEM prog_uchar SPLASH2_MSG[] = "-- Heater Special --";
#endif
#elif MODEL_REPLICATOR2
#ifdef SINGLE_EXTRUDER
const static PROGMEM prog_uchar SPLASH1_MSG[]  = "Sailfish Replicator2";
#else
const static PROGMEM prog_uchar SPLASH1_MSG[] = "  Sailfish Rep 2X   ";
#endif
const static PROGMEM prog_uchar SPLASH2_MSG[] = "--------------------";
#else
#warning "*** Compiling without MODEL_x defined ***"
const static PROGMEM prog_uchar SPLASH1_MSG[] = "      Sailfish      ";
const static PROGMEM prog_uchar SPLASH2_MSG[] = "      --------      ";
#endif
const static PROGMEM prog_uchar SPLASH3_MSG[] = "Thing 32084 r" SVN_VERSION_STR "  ";
const static PROGMEM prog_uchar SPLASH4_MSG[] = "Firmware Version " VERSION_STR;

#ifdef STACK_PAINT
const static PROGMEM prog_uchar SPLASH_SRAM_MSG[] = "Free SRAM ";
#endif

const static PROGMEM prog_uchar SPLASH1A_MSG[] = "    FAIL!           ";
const static PROGMEM prog_uchar SPLASH2A_MSG[] = "    SUCCESS!        ";
const static PROGMEM prog_uchar SPLASH3A_MSG[] = "connected correctly ";
const static PROGMEM prog_uchar SPLASH4A_MSG[] = "Heaters are not     ";

const static PROGMEM prog_uchar GO_MSG[] =         "Prechauffer";
const static PROGMEM prog_uchar STOP_MSG[] =       "Refroidir";
const static PROGMEM prog_uchar PLATFORM_MSG[] =   "Platforme";
const static PROGMEM prog_uchar TOOL_MSG[] =       "Extrudeur";
const static PROGMEM prog_uchar NOZZLE_ERROR_MSG[] = "Toolhead Offset Sys " "must be set to NEW  " "to use this utility.";
const static PROGMEM prog_uchar START_TEST_MSG[]=  "I'm going to print  " "a series of lines so" "we can find my      " "nozzle alignment.   ";
const static PROGMEM prog_uchar EXPLAIN1_MSG[] =   "Look for the best   " "matched line in each" "axis set.  Lines are" "numbered 1-13 and...";
const static PROGMEM prog_uchar EXPLAIN2_MSG[] =   "line one is extra   " "long. The Y axis set" "is left on the plate" "and X axis is right.";
const static PROGMEM prog_uchar END_MSG  [] =      "Great!  I've saved  " "these settings and  " "I'll use them to    " "make nice prints!   ";

const static PROGMEM prog_uchar SELECT_MSG[] =     "Selectionnez les    meilleures lignes.";
const static PROGMEM prog_uchar DONE_MSG[]   =     "Fini!";
const static PROGMEM prog_uchar NO_MSG[]   =       "Non";
const static PROGMEM prog_uchar YES_MSG[]  =       "Oui";

const static PROGMEM prog_uchar XAXIS_MSG[] =      "Axe X";
const static PROGMEM prog_uchar YAXIS_MSG[] =      "Axe Y";
const static PROGMEM prog_uchar HEATER_ERROR_MSG[]=  "Les extrudeurs ne   " "chauffent pas.      " "Verifiez les        " "connexions!";
const static PROGMEM prog_uchar STOP_EXIT_MSG[]   = "Quand le filament   " "sort de la buse,    " "appuyez sur M pour  " "quitter.";
const static PROGMEM prog_uchar TIMEOUT_MSG[]  	  = "5 minute timeout has" "elapsed.  Press 'M' " "to exit.";

const static PROGMEM prog_uchar LOAD_RIGHT_MSG[]    = "Mettre a droite"; 
const static PROGMEM prog_uchar LOAD_LEFT_MSG[]     = "Mettre a gauche"; 
const static PROGMEM prog_uchar LOAD_SINGLE_MSG[]   = "Mettre le plastique"; 
const static PROGMEM prog_uchar UNLOAD_SINGLE_MSG[] = "Retirer le plastique"; 
const static PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]  = "Retirer a gauche"; 
const static PROGMEM prog_uchar UNLOAD_LEFT_MSG[]   = "Retirer a droite"; 

const static PROGMEM prog_uchar JOG1_MSG[]  = "    Mode manuel     ";
const static PROGMEM prog_uchar JOG2X_MSG[] = "        X+          ";
const static PROGMEM prog_uchar JOG3X_MSG[] = "      Retour   Y->  ";
const static PROGMEM prog_uchar JOG4X_MSG[] = "        X-          ";
const static PROGMEM prog_uchar JOG3Y_MSG[] = "  <-X Retour  Z->   ";
const static PROGMEM prog_uchar JOG3Z_MSG[] = "  <-Y Retour        ";


const static PROGMEM prog_uchar DISTANCESHORT_MSG[] = "COURT";
const static PROGMEM prog_uchar DISTANCELONG_MSG[] =  "LONG";

#define HEATING_MSG_DEF "Chauffe:"
#define HEATING_MSG_LEN (sizeof(HEATING_MSG_DEF)-1)
const static PROGMEM prog_uchar HEATING_MSG[] =        HEATING_MSG_DEF;
// HEATING_SPACES_MSG is a full LCD screen wide (20 spaces)
const static PROGMEM prog_uchar HEATING_SPACES_MSG[] = HEATING_MSG_DEF "            ";

const static PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
const static PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "Tete Droite ---/---C";
const static PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "Tete Gauche ---/---C";
const static PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Plateforme  ---/---C";
const static PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Extrudeur:  ---/---C";


const static PROGMEM prog_uchar EXTRUDER_SPACES_MSG[]  = "Extruder           ";
const static PROGMEM prog_uchar RIGHT_SPACES_MSG[]     = "Tete de droite     ";
const static PROGMEM prog_uchar LEFT_SPACES_MSG[]      = "Tete de gauche     ";
const static PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Platforme          ";
const static PROGMEM prog_uchar RESET1_MSG[]           = "Retour parametres";
const static PROGMEM prog_uchar RESET2_MSG[]           = "d'usine?";
const static PROGMEM prog_uchar CANCEL_MSG[] = "Annuler impression?";
const static PROGMEM prog_uchar CANCEL_FIL_MSG[] = "Cancel load/unload?";

const static PROGMEM prog_uchar PAUSE_MSG[]      = "Pause    "; // must be same length as the next msg
const static PROGMEM prog_uchar UNPAUSE_MSG[]    = "Reprendre"; // must be same length as the previous msg
const static PROGMEM prog_uchar COLD_PAUSE_MSG[] = "Cold Pause";

const static PROGMEM prog_uchar BUILDING_MSG[]    = "I'm already building";
const static PROGMEM prog_uchar CARDCOMMS_MSG[]   = "SD card reads are   " "not going well.  Try" "a different card";
const static PROGMEM prog_uchar CARDOPENERR_MSG[] = "Cannot open the file";
const static PROGMEM prog_uchar CARDNOFILES_MSG[] = "SD card is empty";
const static PROGMEM prog_uchar NOCARD_MSG[]      = "SD card not present";
const static PROGMEM prog_uchar CARDERROR_MSG[]   = "SD card read error";
const static PROGMEM prog_uchar CARDCRC_MSG[]     = "SD card read error. " "Too many CRC errors." "Bad card contacts or" "electrical noise.";
const static PROGMEM prog_uchar CARDFORMAT_MSG[]  = "Unable to read this " "SD card format.     " "Reformat as FAT-16. ";
const static PROGMEM prog_uchar CARDSIZE_MSG[]    = "Unable to read SD   " "card partitions over" "2GB in size.        " "Reformat as FAT-16.";
const static PROGMEM prog_uchar EXTRUDER_TIMEOUT_MSG[] = "I timed out while   attempting to heat  my extruder."; 
const static PROGMEM prog_uchar PLATFORM_TIMEOUT_MSG[] = "I timed out while   attempting to heat  my platform.";

const static PROGMEM prog_uchar BUILD_MSG[] =            "Imprimer depuis SD";
const static PROGMEM prog_uchar PREHEAT_MSG[] =          "Prechauffage";
const static PROGMEM prog_uchar UTILITIES_MSG[] =        "Utilitaires";
const static PROGMEM prog_uchar MONITOR_MSG[] =          "Visu Temp   ";
const static PROGMEM prog_uchar JOG_MSG[]   =            "Mode Manuel";
const static PROGMEM prog_uchar CALIBRATION_MSG[] =      "Calibration des axes";
const static PROGMEM prog_uchar HOME_AXES_MSG[] =        "Axes a l'origine";
const static PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Options du filament";
const static PROGMEM prog_uchar VERSION_MSG[] = 		     "Numero de Version";
const static PROGMEM prog_uchar DSTEPS_MSG[] =           "Debloquer Moteurs";
const static PROGMEM prog_uchar ESTEPS_MSG[] =           "Bloquer Moteurs  ";
const static PROGMEM prog_uchar PLATE_LEVEL_MSG[] =      "Relever Plateforme";
const static PROGMEM prog_uchar LED_BLINK_MSG[] =        "Clignotement LED ";
const static PROGMEM prog_uchar LED_STOP_MSG[] =         "Stop Clignotement";
const static PROGMEM prog_uchar PREHEAT_SET_MSG[] =      "Param. prechauffage ";
const static PROGMEM prog_uchar SETTINGS_MSG[] =         "Parametres Generaux";
const static PROGMEM prog_uchar RESET_MSG[] =            "RAZ usine";
const static PROGMEM prog_uchar NOZZLES_MSG[] =          "Calibration Buses";
const static PROGMEM prog_uchar BOT_STATS_MSG[] =        "Bot Statistics";
const static PROGMEM prog_uchar TOOL_COUNT_MSG[] =       "Nb Tetes:";
const static PROGMEM prog_uchar SOUND_MSG[] =            "Son";
const static PROGMEM prog_uchar LED_MSG[] =              "Eclairage";
const static PROGMEM prog_uchar LED_HEAT_MSG[] =         "Coul Chauf";
const static PROGMEM prog_uchar EXIT_MSG[] =             "Sortir du Menu";
const static PROGMEM prog_uchar ACCELERATE_MSG[] = 	     "Accelerer";
const static PROGMEM prog_uchar HBP_MSG[]                 = "HBP installed";
const static PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[] = "Override GcTemp";
const static PROGMEM prog_uchar PAUSE_HEAT_MSG[]	        = "Pause with Heat";
const static PROGMEM prog_uchar EXTRUDER_HOLD_MSG[]       = "Extruder Hold";
const static PROGMEM prog_uchar TOOL_OFFSET_SYSTEM_MSG[]  = "Tool Offset Sys";
const static PROGMEM prog_uchar SD_USE_CRC_MSG[]          = "Check SD reads";
const static PROGMEM prog_uchar PSTOP_ENABLE_MSG[]        = "P-Stop control";
const static PROGMEM prog_uchar OLD_MSG[]                 = "OLD";
const static PROGMEM prog_uchar NEW_MSG[]                 = "NEW";
const static PROGMEM prog_uchar DISABLED_MSG[]            = "N/A";
#ifdef DITTO_PRINT
const static PROGMEM prog_uchar DITTO_PRINT_MSG[]         = "Ditto Printing";
#endif
const static PROGMEM prog_uchar PAUSEATZPOS_MSG[]	        = "Pause at ZPos";
const static PROGMEM prog_uchar CHANGE_SPEED_MSG[]        = "Change Speed";
const static PROGMEM prog_uchar CHANGE_TEMP_MSG[]         = "Change Temperature";
const static PROGMEM prog_uchar FAN_ON_MSG[]              = "Set Cooling Fan ON "; // Needs trailing space
const static PROGMEM prog_uchar FAN_OFF_MSG[]             = "Set Cooling Fan OFF";
const static PROGMEM prog_uchar RETURN_TO_MAIN_MSG[]      = "Main Menu";
const static PROGMEM prog_uchar PRINT_ANOTHER_MSG[]       = "Print Another Copy";
const static PROGMEM prog_uchar CANNOT_PRINT_ANOTHER_MSG[]= "(Cannot Print Copy)";


const static PROGMEM prog_uchar RED_COLOR_MSG[]    = "ROUGE ";
const static PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE";
const static PROGMEM prog_uchar PINK_COLOR_MSG[]   = "ROSE  ";
const static PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "VERT  ";
const static PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLEU  ";
const static PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "VIOLET";
const static PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "BLANC ";
const static PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "AUTRE ";
const static PROGMEM prog_uchar OFF_COLOR_MSG[]    = "OFF   ";

const static PROGMEM prog_uchar ERROR_MSG[] =   "erreur";
const static PROGMEM prog_uchar NA_MSG[] =      "  NA    ";
const static PROGMEM prog_uchar WAITING_MSG[] = "attente ";

const static PROGMEM prog_uchar EXTEMP_CHANGE_MSG[]               = "My temperature was  " "changed externally. " "Reselect filament   " "menu to try again.";
const static PROGMEM prog_uchar HEATER_INACTIVITY_MSG[]           = "Exctincion des tetes" "due a l'inactivite";
const static PROGMEM prog_uchar HEATER_TOOL_MSG[]                 = "Extruder Failure!   ";
const static PROGMEM prog_uchar HEATER_TOOL0_MSG[]                = "Tool 0 Failure!     ";
const static PROGMEM prog_uchar HEATER_TOOL1_MSG[]                = "Tool 1 Failure!     ";
const static PROGMEM prog_uchar HEATER_PLATFORM_MSG[]             = "Platform Failure!   ";

//The following messages are in english because the french translation is too large
//and results in a firmware image that does not fit in the available flash memory space
//Please try to save bytes maybe by using some abreviations
const static PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[] = "Temp limit reached. " "Shutdown or restart.";
const static PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[]     = "Not heating properly" "Check wiring.";
const static PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[]   = "Temperature dropping" "Check wiring.";
const static PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[]  = "Temperature reads   " "are failing.        " "Check wiring.";
//const static PROGMEM prog_uchar HEATER_FAIL_SOFTWARE_CUTOFF_MSG[] = "Surchauffe des tetes" "Temperature limite  " "Atteinte! Veuillez  " "Redemarrer";
//const static PROGMEM prog_uchar HEATER_FAIL_NOT_HEATING_MSG[]     = "Surchauffe des tetes" "Elles ne chauffent  " "pas correctement !  " "Verif. connectiques ";
//const static PROGMEM prog_uchar HEATER_FAIL_DROPPING_TEMP_MSG[]   = "Echec du chauffage !" "La temperature des  " "tetes chute !       " "Verif. connectiques ";
//const static PROGMEM prog_uchar HEATER_FAIL_NOT_PLUGGED_IN_MSG[]  = "Erreur de chauffe ! " "Echec du releve de  " "temperature!        " "Verif. connectiques ";
const static PROGMEM prog_uchar HEATER_FAIL_READ_MSG[]            = "Temperature reads   " "out of range.       " "Check wiring.";

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
const static PROGMEM prog_uchar STATS_MSG[]			= "Print Statistics";
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

#endif // __MENU_FR_HH__
