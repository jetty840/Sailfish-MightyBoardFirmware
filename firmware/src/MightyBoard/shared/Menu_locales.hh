#ifndef __MENU__LOCALES__
#define __MENU__LOCALES__

#include <avr/pgmspace.h>
#include <string.h>

const static PROGMEM prog_uchar ON_CELCIUS_MSG[] = "/   C";
const static PROGMEM prog_uchar CELCIUS_MSG[] =    "C    ";
const static PROGMEM prog_uchar BLANK_CHAR_MSG[] = " ";
const static PROGMEM prog_uchar BLANK_CHAR_4_MSG[] = "    ";

const static PROGMEM prog_uchar CLEAR_MSG[]     =  "                    ";

#define LEVEL_PLATE const static uint8_t LevelPlate[] PROGMEM = { 137,  16,  153,  0,  0,  0,  0,  82,  101,  112,  71,  32,  66,  117,  105,  108,  100,  0,  150,  0,  255,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  155,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  165,  28,  0,  0,  24,  0,  0,  160,  64,  149,  4,  131,  4,  220,  5,  0,  0,  20,  0,  144,  31,  139,  0,  0,  0,  0,  0,  0,  0,  0,  160,  15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  0,  0,  0,  155,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  165,  28,  0,  0,  24,  0,  0,  32,  65,  149,  4,  137,  27,  149,  6,  0,  0,  0,  80,  114,  101,  115,  115,  32,  34,  77,  34,  32,  119,  104,  101,  110,  32,  108,  101,  118,  101,  108,  0,  137,  31,  137,  31 };

#define LEVEL_PLATE_LEN 189

#if defined(MODEL_REPLICATOR)
#if defined(FF_CREATOR)
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish FF Creator ";
#elif defined(FF_CREATOR_X)
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish FF CreatorX";
#elif defined(WANHAO_DUP4)
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Wanhao Duplicator 4 ";
#elif defined(CORE_XY_STEPPER)
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish Rep CoreXYs";
#elif defined(CORE_XY)
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish Rep1 CoreXY";
#else
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish Replicator1";
#endif
#if !defined(HEATERS_ON_STEROIDS) || defined(FF_CREATOR) || defined(FF_CREATOR_X) || defined(WANHAO_DUP4)
const static PROGMEM prog_uchar SPLASH2_MSG[] = "--------------------";
#else
const static PROGMEM prog_uchar SPLASH2_MSG[] = "-- Heater Special --";
#endif
#elif defined(MODEL_REPLICATOR2)
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


#include "locale.hh"

#endif // __MENU__LOCALES__
