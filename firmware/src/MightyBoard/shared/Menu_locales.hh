#ifndef __MENU__LOCALES__
#define __MENU__LOCALES__

#include <avr/pgmspace.h>

#define FRENCH 1

#if LOCALE == FRENCH
static PROGMEM prog_uchar SPLASH3_MSG[] = "                    ";
static PROGMEM prog_uchar SPLASH1_MSG[] = "   La Replicator    ";
static PROGMEM prog_uchar SPLASH2_MSG[] = "    ----------      ";
static PROGMEM prog_uchar SPLASH4_MSG[] = "Firmware Version b. ";
#else // ENGLISH
static PROGMEM prog_uchar SPLASH1_MSG[] = "  The Replicator    ";
static PROGMEM prog_uchar SPLASH2_MSG[] = "    ----------      ";
static PROGMEM prog_uchar SPLASH3_MSG[] = "                    ";
static PROGMEM prog_uchar SPLASH4_MSG[] = "Firmware Version b. ";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar SPLASH1B_MSG[] = "   Heater Test On   ";
static PROGMEM prog_uchar SPLASH2B_MSG[] = "Press Center to Quit";
static PROGMEM prog_uchar SPLASH3B_MSG[] = "  Ce test dure dix  ";
static PROGMEM prog_uchar SPLASH4B_MSG[] = "    secondes        ";
#else // ENGLISH
static PROGMEM prog_uchar SPLASH1B_MSG[] = "   Heater Test On   ";   // XXX old name: splash1
static PROGMEM prog_uchar SPLASH2B_MSG[] = "Press Center to Quit";   // XXX old name: splash2
static PROGMEM prog_uchar SPLASH3B_MSG[] = "This test takes ten ";   // XXX old name: splash3
static PROGMEM prog_uchar SPLASH4B_MSG[] = "    seconds         ";   // XXX old name: splash4
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar SPLASH1A_MSG[] = "     ECHEC          ";
static PROGMEM prog_uchar SPLASH2A_MSG[] = "    SUCCES!         ";
static PROGMEM prog_uchar SPLASH3A_MSG[] = "     connectes      ";
static PROGMEM prog_uchar SPLASH4A_MSG[] = "Chauffages mal      ";
static PROGMEM prog_uchar SPLASH5A_MSG[] = "                    ";	
#else // ENGLISH
static PROGMEM prog_uchar SPLASH1A_MSG[] = "    FAIL!           ";
static PROGMEM prog_uchar SPLASH2A_MSG[] = "    SUCCESS!        ";
static PROGMEM prog_uchar SPLASH3A_MSG[] = "connected correctly ";
static PROGMEM prog_uchar SPLASH4A_MSG[] = "Heaters are not     ";
static PROGMEM prog_uchar SPLASH5A_MSG[] = "                    ";	
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar GO_MSG[] = "     Commencer           prechauffage   ";
static PROGMEM prog_uchar STOP_MSG[] = "Stopper prechauffage";
static PROGMEM prog_uchar RIGHT_TOOL_MSG[] = "Extrudeur de droite";
static PROGMEM prog_uchar LEFT_TOOL_MSG[] = "Extrudeur de gauche";
static PROGMEM prog_uchar PLATFORM_MSG[] = "Platforme";
static PROGMEM prog_uchar TOOL_MSG[] = "Extrudeur";
#else // ENGLISH
static PROGMEM prog_uchar GO_MSG[] = "Start Preheat!";
static PROGMEM prog_uchar STOP_MSG[] = "Stop Preheat!";
static PROGMEM prog_uchar RIGHT_TOOL_MSG[] = "Right Tool";
static PROGMEM prog_uchar LEFT_TOOL_MSG[] = "Left Tool";
static PROGMEM prog_uchar PLATFORM_MSG[] = "Platform";
static PROGMEM prog_uchar TOOL_MSG[] = "Extruder";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar START_MSG[] =    "Bonjour !           Je suis Replicator     Appuyez sur le M rouge pour commencer";
static PROGMEM prog_uchar BUTTONS1_MSG[] = "Le 'M' clignotant   signifie en attente de commande pour    continuer.          ";
static PROGMEM prog_uchar BUTTONS2_MSG[] = "Le 'M' fixe signifieque je travaille.   De nouvelles infos  quand travail fini. ";
static PROGMEM prog_uchar EXPLAIN_MSG[] =  "L'etape suivante estla configuration.   D'abord restaurer laplateforme          ";                                  
static PROGMEM prog_uchar LEVEL_MSG[] =    "d'impression.       Elle est trop haute actuellement.                           ";
static PROGMEM prog_uchar BETTER_MSG[] =   "La plateforme est   en bonne place.     Mettons du          plastique.          ";
static PROGMEM prog_uchar TRYAGAIN_MSG[] = "Essayez encore...                                                               ";
static PROGMEM prog_uchar GO_ON_MSG[]   =  "Mettez du           plastique maintenantPour aide voir      makerbot.com/help   ";     
static PROGMEM prog_uchar SD_MENU_MSG[] =  "Parfait !           Allez dans le menu  de la carte SD      et choisir un objet.";
static PROGMEM prog_uchar FAIL_MSG[] =     "Allons au menu      principal. Pour aidevoir                makerbot.com/help   ";
static PROGMEM prog_uchar START_TEST_MSG[] =    "Impression de test  pour regler         l'alignement des    buses.              ";
static PROGMEM prog_uchar EXPLAIN1_MSG[] = "Choisir le meilleur resultat pour chaqueaxe.Les lignes vont de 1 a 13 et        ";
static PROGMEM prog_uchar EXPLAIN2_MSG[] = "la ligne 1 est plus longue.             Axe Y est a gauche  et axe X a droite.  ";
static PROGMEM prog_uchar END_MSG [] =    "Choix enregistres.  Imprimez de beaux   objets !                                ";
#else // ENGLISH
static PROGMEM prog_uchar START_MSG[] =    "Welcome!            I'm the Replicator. Press the red M to  get started!        ";
static PROGMEM prog_uchar BUTTONS1_MSG[] = "A blinking 'M' meansI'm waiting and willcontinue when you   press the button... ";
static PROGMEM prog_uchar BUTTONS2_MSG[] = "A solid 'M' means   I'm working and willupdate my status    when I'm finished...";
static PROGMEM prog_uchar EXPLAIN_MSG[] =  "Our next steps will get me set up to    print! First, we'll restore my build... ";                                  
static PROGMEM prog_uchar LEVEL_MSG[] =    "platform so it's    nice and level. It'sprobably a bit off  from shipping...    ";
static PROGMEM prog_uchar BETTER_MSG[] =   "Aaah, that feels    much better.        Let's go on and loadsome plastic!       ";
static PROGMEM prog_uchar TRYAGAIN_MSG[] = "We'll try again!                                                                ";
static PROGMEM prog_uchar GO_ON_MSG[]   =  "We'll keep going    and load some       plastic! For help goto makerbot.com/help";     
static PROGMEM prog_uchar SD_MENU_MSG[] =  "Awesome!            We'll go to the SD  card Menu and you   can select a print! ";
static PROGMEM prog_uchar FAIL_MSG[] =     "We'll go to the mainmenu. If you need   help go to:         makerbot.com/help   ";
static PROGMEM prog_uchar START_TEST_MSG[] =    "I'm going to print  a series of lines sowe can find my      nozzle alignment.   "; // XXX old name: start[]
static PROGMEM prog_uchar EXPLAIN1_MSG[] = "Look for the best   matched line in eachaxis set. Lines are numbered 1-13 and...";
static PROGMEM prog_uchar EXPLAIN2_MSG[] = "line one is extra   long. The Y axis setis left on the plateand X axis is right.";
static PROGMEM prog_uchar END_MSG  [] =    "Great!  I've saved  these settings and  I'll use them to    make nice prints!   ";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar SELECT_MSG[] = "Selectionnez les    meilleures lignes.  ";
static PROGMEM prog_uchar DONE_MSG[]   = "Fini!";
static PROGMEM prog_uchar NO_MSG[]   =   "Non";
static PROGMEM prog_uchar YES_MSG[]  =   "Oui";
#else // ENGLISH
static PROGMEM prog_uchar SELECT_MSG[] = "Select best lines.";
static PROGMEM prog_uchar DONE_MSG[]   = "Done!";
static PROGMEM prog_uchar NO_MSG[]   =   "No";
static PROGMEM prog_uchar YES_MSG[]  =   "Yes!";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar XAXIS_MSG[] = "Axe X";
static PROGMEM prog_uchar YAXIS_MSG[] = "Axe Y";
#else // ENGLISH
static PROGMEM prog_uchar XAXIS_MSG[] = "X Axis Line";
static PROGMEM prog_uchar YAXIS_MSG[] = "Y Axis Line";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar HEATER_ERROR_MSG[]= "Les extrudeurs ne   chauffent pas.      Verifiez les        connexions !        ";
static PROGMEM prog_uchar EXPLAIN_ONE_MSG[] = "Appuyer sur la baguegrise en haut des   extrudeurs et       tirez sur le guide. ";
static PROGMEM prog_uchar EXPLAIN_TWO_MSG[] = "Envoyez du filament dans le tube depuis l'arriere jusqu'a   ce qu'il            ";
static PROGMEM prog_uchar EXPLAIN_ONE_S_MSG[]= "Appuyer sur la baguegrise en haut de    l'extrudeur et      retirez le guide.   ";
static PROGMEM prog_uchar EXPLAIN_TWO_S_MSG[]= "Envoyez du filament dans le tube depuis l'arriere jusqu'a   ce qu'il            ";
static PROGMEM prog_uchar EXPLAIN_THRE_MSG[]= "ressorte devant.    Extrudeur en chauffeChargement du       filament en cours...";
static PROGMEM prog_uchar EXPLAIN_FOUR_MSG[]= "Ceci peut durer     quelques minutes.   ATTENTION a ne pas  vous bruler !       ";
#else // ENGLISH
static PROGMEM prog_uchar HEATER_ERROR_MSG[]= "My extruders are    not heating up.     Check my            connections!        ";
static PROGMEM prog_uchar EXPLAIN_ONE_MSG[] = "Press down on the   grey rings at top ofthe extruders and   pull the black...   ";
static PROGMEM prog_uchar EXPLAIN_TWO_MSG[] = "guide tubes out. Nowfeed filament from  the back through thetubes until it...   ";
static PROGMEM prog_uchar EXPLAIN_ONE_S_MSG[]= "Press down on the   grey ring at top of the extruder and    pull the black...   ";
static PROGMEM prog_uchar EXPLAIN_TWO_S_MSG[]= "guide tube out.  Nowfeed filament from  the back through thetube until it...   ";
static PROGMEM prog_uchar EXPLAIN_THRE_MSG[]= "pops out in front.  I'm heating up my   extruder so we can  load the filament...";
static PROGMEM prog_uchar EXPLAIN_FOUR_MSG[]= "This might take a   few minutes.        And watch out, the  nozzle will get HOT!";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar HEATING_BAR_MSG[] = "Progression du      chauffage :                                                 ";
static PROGMEM prog_uchar heating[] 	= "Mise en temperature de l'extrudeur.     Merci de patienter.                     ";
static PROGMEM prog_uchar READY_RIGHT_MSG[] = "Replicator prete.   Chargez du filament dans l'extrudeur de droite.             ";
static PROGMEM prog_uchar READY_SINGLE_MSG[]= "Replicator prete.   Chargez du filament dans le tube noir del'extrudeur         "; 
static PROGMEM prog_uchar READY_REV_MSG[]   = "Replicator prete.   Detachez le guide etretirez doucement le filament...        ";
static PROGMEM prog_uchar READY_LEFT_MSG[]  = "Chargez maintenant  du filament dans    l'extrudeur gauche.                     ";
static PROGMEM prog_uchar TUG_MSG[]         = "via la bague grise  jusqu'a ce que le   moteur aggripe      le  fil.            ";
static PROGMEM prog_uchar STOP_MSG_MSG[]        = "Quand le filament   sort de la buse,    appuyez sur M pour  stopper l'extrudeur."; 
static PROGMEM prog_uchar STOP_EXIT_MSG[]   = "Quand le filament   sort de la buse,    appuyez sur M pour  quitter           . "; 
static PROGMEM prog_uchar STOP_REVERSE_MSG[]= "Quand le filament   est totalement sortiappuyez sur M pour  quitter           . ";            
static PROGMEM prog_uchar PUSH_HARDER_MSG[]    = "Vous devriez        ressayer en      appuyant plus fort                      ";                                                      
static PROGMEM prog_uchar KEEP_GOING_MSG[]       = "Continuons !        Besoin d'aide ?     Allez sur           makerbot.com/help   ";  
static PROGMEM prog_uchar FINISH_MSG[]      = "Arret extrudeur en  cours. Appuyez sur Mpour continuer.                         ";  
static PROGMEM prog_uchar GO_ON_LEFT_MSG[]  = "Continuons !        Chargez du filament dans l'extrudeur    gauche...           ";
#else // ENGLISH
static PROGMEM prog_uchar HEATING_BAR_MSG[] = "Heating Progress:                                                               ";
static PROGMEM prog_uchar heating[] 	= "I'm heating up my   extruder!           Please wait.                            ";
static PROGMEM prog_uchar READY_RIGHT_MSG[] = "OK I'm ready!       First we'll load theright extruder.     Push filament in... ";
static PROGMEM prog_uchar READY_SINGLE_MSG[]= "OK I'm ready!       Pop the guide tube  off and push the    filament down...    "; 
static PROGMEM prog_uchar READY_REV_MSG[]   = "OK I'm ready!       Pop the guide tube  off and pull        filament gently...  ";
static PROGMEM prog_uchar READY_LEFT_MSG[]  = "Great! Now we'll    load the left       extruder. Push      filament down...    ";
static PROGMEM prog_uchar TUG_MSG[]         = "through the grey    ring until you feel the motor tugging   the plastic in...   ";
static PROGMEM prog_uchar STOP_MSG_MSG[]        = "When filament is    extruding out of thenozzle, Press 'M'   to stop extruding.  ";  // XXX old name: stop[]
static PROGMEM prog_uchar STOP_EXIT_MSG[]   = "When filament is    extruding out of thenozzle, Press 'M'   to exit             "; 
static PROGMEM prog_uchar STOP_REVERSE_MSG[]= "When my filament is released,           Press 'M' to exit.                      ";            
static PROGMEM prog_uchar PUSH_HARDER_MSG[]    = "OK! I'll keep my    motor running. You  may need to push    harder...           ";  // XXX old name: tryagain[]
static PROGMEM prog_uchar KEEP_GOING_MSG[]       = "We'll keep going.   If you're having    trouble, check out  makerbot.com/help   ";  // XXX old name: go_on[]
static PROGMEM prog_uchar FINISH_MSG[]      = "Great!  I'll stop   running my extruder.Press M to continue.                    ";  
static PROGMEM prog_uchar GO_ON_LEFT_MSG[]  = "We'll keep going.   Lets try the left   extruder. Push      filament down...    ";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar READY1_MSG[] = "Tout va bien ?     ";
static PROGMEM prog_uchar READY2_MSG[] = "Lancer impression ?";
static PROGMEM prog_uchar NOZZLE_MSG_MSG[] = "Does my nozzle";
static PROGMEM prog_uchar HEIGHT_CHK_MSG[] = "height check out?";
static PROGMEM prog_uchar HEIGHT_GOOD_MSG[]   =   "Bonne hauteur !   ";   // XXX old name: yes[]
static PROGMEM prog_uchar TRY_AGAIN_MSG[]  =   "Recommencons.   ";       // XXX old name: no[]
#else // ENGLISH
static PROGMEM prog_uchar READY1_MSG[] = "How'd it go? Ready ";
static PROGMEM prog_uchar READY2_MSG[] = "to try a print?    ";
static PROGMEM prog_uchar NOZZLE_MSG_MSG[] = "Does my nozzle";        // XXX old name: ready1[]
static PROGMEM prog_uchar HEIGHT_CHK_MSG[] = "height check out?";     // XXX old name: ready2[]
static PROGMEM prog_uchar HEIGHT_GOOD_MSG[]   =   "My height is good!";   // XXX old name: yes[]
static PROGMEM prog_uchar TRY_AGAIN_MSG[]  =   "Let's try again.";       // XXX old name: no[]
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar QONE_MSG[] = "Du plastique sort-il";
static PROGMEM prog_uchar QTWO_MSG[] = "de l'extrudeur ?    ";
static PROGMEM prog_uchar LOAD_MSG[] =   "Mettre     "; 
static PROGMEM prog_uchar UNLOAD_MSG[] = "Retirer    "; 
#else // ENGLISH
static PROGMEM prog_uchar QONE_MSG[] = "Did plastic extrude ";
static PROGMEM prog_uchar QTWO_MSG[] = "from the nozzle?";
static PROGMEM prog_uchar LOAD_MSG[] = "Load"; 
static PROGMEM prog_uchar UNLOAD_MSG[] = "UnLoad"; 
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar JOG1_MSG[]  = "    Mode manuel     ";
static PROGMEM prog_uchar JOG2X_MSG[] = "        X+          ";
static PROGMEM prog_uchar JOG3X_MSG[] = "      Retour   Y->  ";
static PROGMEM prog_uchar JOG4X_MSG[] = "        X-          ";
static PROGMEM prog_uchar JOG2Y_MSG[] = "        Y+          ";
static PROGMEM prog_uchar JOG3Y_MSG[] = "  <-X Retour  Z->   ";
static PROGMEM prog_uchar JOG4Y_MSG[] = "        Y-          ";
static PROGMEM prog_uchar JOG2Z_MSG[] = "        Z-          ";
static PROGMEM prog_uchar JOG3Z_MSG[] = "  <-Y Retour        ";
static PROGMEM prog_uchar JOG4Z_MSG[] = "        Z+          ";
#else // ENGLISH
static PROGMEM prog_uchar JOG1_MSG[]  = "     Jog mode       ";
static PROGMEM prog_uchar JOG2X_MSG[] = "        X+          ";
static PROGMEM prog_uchar JOG3X_MSG[] = "      (Back)   Y->  ";
static PROGMEM prog_uchar JOG4X_MSG[] = "        X-          ";
static PROGMEM prog_uchar JOG2Y_MSG[] = "        Y+          ";
static PROGMEM prog_uchar JOG3Y_MSG[] = "  <-X (Back)  Z->   ";
static PROGMEM prog_uchar JOG4Y_MSG[] = "        Y-          ";
static PROGMEM prog_uchar JOG2Z_MSG[] = "        Z-          ";
static PROGMEM prog_uchar JOG3Z_MSG[] = "  <-Y (Back)        ";
static PROGMEM prog_uchar JOG4Z_MSG[] = "        Z+          ";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar DISTANCESHORT_MSG[] = "COURT";
static PROGMEM prog_uchar DISTANCELONG_MSG[] = "LONG";
static PROGMEM prog_uchar GAMEOVER_MSG[] =  "PERDU !";
#else // ENGLISH
static PROGMEM prog_uchar DISTANCESHORT_MSG[] = "SHORT";
static PROGMEM prog_uchar DISTANCELONG_MSG[] = "LONG";
static PROGMEM prog_uchar GAMEOVER_MSG[] =  "GAME OVER!";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar HEATING_MSG[] =  "Chauffe:";
static PROGMEM prog_uchar HEATING_SPACES_MSG[] = "Chauffe:            ";
#else // ENGLISH
static PROGMEM prog_uchar HEATING_MSG[] =  "Heating:";
static PROGMEM prog_uchar HEATING_SPACES_MSG[] = "Heating:            ";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
static PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "Tete Droite ---/---C";
static PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "Tete Gauche ---/---C";
static PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Plateforme  ---/---C";
static PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Extrudeur:  ---/---C";
#else // ENGLISH
static PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
static PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "R Extruder: ---/---C";
static PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "L Extruder: ---/---C";
static PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Platform:   ---/---C";
static PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Extruder:   ---/---C";
#endif

static PROGMEM prog_uchar CLEAR_MSG[] =  "                    ";
static PROGMEM prog_uchar BLANKLINE_MSG[] =  "                ";

#if LOCALE == FRENCH
static PROGMEM prog_uchar SET1_MSG[] = "Param. prechauffage ";
static PROGMEM prog_uchar RIGHT_SPACES_MSG[] =  "Tete de droite     ";
static PROGMEM prog_uchar LEFT_SPACES_MSG[]   = "Tete de gauche     ";
static PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Platforme       ";
static PROGMEM prog_uchar RESET_SETTINGS_MSG[] = "  Revenir aux valeurs";
static PROGMEM prog_uchar SET2_MSG[] = "     d'usine ?      ";
static PROGMEM prog_uchar CANCEL_MSG[] = "      Annuler       cette impression ?";
static PROGMEM prog_uchar CANCEL_PROCESS_MSG[] = "Quitter processus ?";
#else // ENGLISH
static PROGMEM prog_uchar SET1_MSG[] = "Preheat Settings    ";
static PROGMEM prog_uchar RIGHT_SPACES_MSG[] = "Right Tool          ";    // XXX old name: right[]
static PROGMEM prog_uchar LEFT_SPACES_MSG[]   = "Left Tool           ";   // XXX old name: left[]
static PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Platform        ";    // XXX old name: platform[]
static PROGMEM prog_uchar RESET_SETTINGS_MSG[] = "Reset Settings to ";       // XXX old name: set1[]
static PROGMEM prog_uchar SET2_MSG[] = "Default values?";
static PROGMEM prog_uchar CANCEL_MSG[] = "Cancel this print?";
static PROGMEM prog_uchar CANCEL_PROCESS_MSG[] = "Quit this process?";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar PAUSE_MSG[] = "Pause";
static PROGMEM prog_uchar UNPAUSE_MSG[] = "Reprendre";
#else // ENGLISH
static PROGMEM prog_uchar PAUSE_MSG[] = "Pause";
static PROGMEM prog_uchar UNPAUSE_MSG[] = "UnPause";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar BUILD_MSG[] =   "Imprimer depuis SD";
static PROGMEM prog_uchar PREHEAT_MSG[] = "Prechauffage";
static PROGMEM prog_uchar UTILITIES_MSG[] = "Utilitaires";
static PROGMEM prog_uchar MONITOR_MSG[] = "Monitor Mode";
static PROGMEM prog_uchar JOG_MSG[]   =   "Mode Manuel";
static PROGMEM prog_uchar CALIBRATION_MSG[] = "Calibration des axes";
static PROGMEM prog_uchar HOME_AXES_MSG[] = "Home Axes";
static PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Options du filament";
static PROGMEM prog_uchar STARTUP_MSG[] = "Script de demarrage.";
static PROGMEM prog_uchar DSTEPS_MSG[] = "Debloquer Moteurs";
static PROGMEM prog_uchar ESTEPS_MSG[] = "Bloquer Moteurs  ";
static PROGMEM prog_uchar PLATE_LEVEL_MSG[] = "Relever plateforme";
static PROGMEM prog_uchar LED_BLINK_MSG[] = "Clignotement LED ";
static PROGMEM prog_uchar LED_STOP_MSG[] = "Stop clignotement";
static PROGMEM prog_uchar PREHEAT_SETTINGS_MSG[] = "Param. prechauffage ";
static PROGMEM prog_uchar SETTINGS_MSG[] = "Parametres generaux";
static PROGMEM prog_uchar RESET_MSG[] =    "RAZ usine";
static PROGMEM prog_uchar NOZZLES_MSG[] = "Calibration Buses";
static PROGMEM prog_uchar TOOL_COUNT_MSG[]   =   "Nb Tetes:  ";
static PROGMEM prog_uchar SOUND_MSG[] =       "Son";
static PROGMEM prog_uchar LED_MSG[] =             "Eclairage     ";
static PROGMEM prog_uchar LED_HEAT_MSG[] = "Coul Chauf";
static PROGMEM prog_uchar HELP_SCREENS_MSG[] = "Texte d'aide";
static PROGMEM prog_uchar EXIT_MSG[] =   "sortir du menu";
#else // ENGLISH
static PROGMEM prog_uchar BUILD_MSG[] =   "Print from SD";
static PROGMEM prog_uchar PREHEAT_MSG[] = "Preheat";
static PROGMEM prog_uchar UTILITIES_MSG[] = "Utilities";
static PROGMEM prog_uchar MONITOR_MSG[] = "Monitor Mode";
static PROGMEM prog_uchar JOG_MSG[]   =   "Jog Mode";
static PROGMEM prog_uchar CALIBRATION_MSG[] = "Calibrate Axes";
static PROGMEM prog_uchar HOME_AXES_MSG[] = "Home Axes";
static PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Filament Options";
static PROGMEM prog_uchar STARTUP_MSG[] = "Run Startup Script";
static PROGMEM prog_uchar DSTEPS_MSG[] = "Disable Steppers";
static PROGMEM prog_uchar ESTEPS_MSG[] = "Enable Steppers  ";
static PROGMEM prog_uchar PLATE_LEVEL_MSG[] = "Level Build Plate";
static PROGMEM prog_uchar LED_BLINK_MSG[] = "Blink LEDs       ";
static PROGMEM prog_uchar LED_STOP_MSG[] = "Stop Blinking!";
static PROGMEM prog_uchar PREHEAT_SETTINGS_MSG[] = "Preheat Settings";
static PROGMEM prog_uchar SETTINGS_MSG[] = "General Settings";
static PROGMEM prog_uchar RESET_MSG[] = "Restore Defaults";
static PROGMEM prog_uchar NOZZLES_MSG[] = "Calibrate Nozzles";
static PROGMEM prog_uchar TOOL_COUNT_MSG[]   =   "Tool Count ";
static PROGMEM prog_uchar SOUND_MSG[] =       "Sound";
static PROGMEM prog_uchar LED_MSG[] =             "LED Color     ";
static PROGMEM prog_uchar LED_HEAT_MSG[] = "Heat LEDs";
static PROGMEM prog_uchar HELP_SCREENS_MSG[] = "Help Text  ";
static PROGMEM prog_uchar EXIT_MSG[] =   "exit menu";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar SDONE_MSG[] =    "Carte SD avec les   ";
static PROGMEM prog_uchar SDTWO_MSG[] =    "scripts MakerBot est";
static PROGMEM prog_uchar SDTHREE_MSG[] =   "requise. Voir       ";
static PROGMEM prog_uchar SDFOUR_MSG[]  =   "makerbot.com/repSD  ";
static PROGMEM prog_uchar NOCARD_MSG[] = "Carte SD introuvable";
#else // ENGLISH
static PROGMEM prog_uchar SDONE_MSG[] =    "An SD card with     ";
static PROGMEM prog_uchar SDTWO_MSG[] =    "MakerBot scripts is ";
static PROGMEM prog_uchar SDTHREE_MSG[] =   "required.  See      ";
static PROGMEM prog_uchar SDFOUR_MSG[]  =   "makerbot.com/repSD  ";
static PROGMEM prog_uchar NOCARD_MSG[] = "No SD card found";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar RED_COLOR_MSG[]    = "ROUGE ";
static PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE ";
static PROGMEM prog_uchar PINK_COLOR_MSG[]   = "ROSE  ";
static PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "VERT  ";
static PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLEU  ";
static PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "VIOLET";
static PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "BLANC ";
static PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "AUTRE ";
#else // ENGLISH
static PROGMEM prog_uchar RED_COLOR_MSG[]    = "RED   ";
static PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE ";
static PROGMEM prog_uchar PINK_COLOR_MSG[]   = "PINK  ";
static PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "GREEN ";
static PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLUE  ";
static PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "PURPLE";
static PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "WHITE ";
static PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "CUSTOM";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar TOOL_SINGLE_MSG[] = "SIMPLE";
static PROGMEM prog_uchar TOOL_DUAL_MSG[] = "DOUBLE";
#else // ENGLISH
static PROGMEM prog_uchar TOOL_SINGLE_MSG[] = "SINGLE";
static PROGMEM prog_uchar TOOL_DUAL_MSG[] = "DUAL  ";
#endif

#if LOCALE == FRENCH
static PROGMEM prog_uchar RIGHT_MSG[] = "Droite";
static PROGMEM prog_uchar LEFT_MSG[] = "Gauche";
static PROGMEM prog_uchar ERROR_MSG[] = "Erreur";

static PROGMEM prog_uchar NA_MSG[] = "  NA    ";
static PROGMEM prog_uchar WAITING_MSG[] = "waiting ";
#else // ENGLISH
static PROGMEM prog_uchar RIGHT_MSG[] = "Right";
static PROGMEM prog_uchar LEFT_MSG[] = "Left";
static PROGMEM prog_uchar ERROR_MSG[] = "error!";
static PROGMEM prog_uchar NA_MSG[] = "  NA    ";
static PROGMEM prog_uchar WAITING_MSG[] = "waiting ";
#endif

//#if LOCALE == FRENCH
//#else // ENGLISH
static PROGMEM prog_uchar ON_MSG[] = "ON ";
static PROGMEM prog_uchar OFF_MSG[] = "OFF";
static PROGMEM prog_uchar ON_CELCIUS_MSG[] = "/   C";
static PROGMEM prog_uchar CELCIUS_MSG[] = "C    ";
//#endif

static PROGMEM prog_uchar ARROW_MSG[] = "-->";
static PROGMEM prog_uchar NO_ARROW_MSG[] = "   ";
static PROGMEM prog_uchar BLANK_CHAR_MSG[] = " ";

#endif // __MENU__LOCALES__
