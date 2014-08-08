#ifndef __MENU__LOCALES__
#define __MENU__LOCALES__

#include <avr/pgmspace.h>
#include <string.h>
#include "Model.hh"

const static PROGMEM prog_uchar ON_CELCIUS_MSG[] = "/   C";
const static PROGMEM prog_uchar CELCIUS_MSG[] =    "C    ";
const static PROGMEM prog_uchar BLANK_CHAR_MSG[] = " ";
const static PROGMEM prog_uchar BLANK_CHAR_4_MSG[] = "    ";

const static PROGMEM prog_uchar CLEAR_MSG[]     =  "                    ";

// Note: RepG is adding an extra disable axes on the end....
//       so check the last four bytes and remove the last two
//       if they are 137, 31, 137, 31
#define LEVEL_PLATE const static uint8_t LevelPlate[] PROGMEM = { 137,  8,  153,  0,  0,  0,  0,  82,  101,  112,  71,  32,  66,  117,  105,  108,  100,  0,  150,  0,  255,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  155,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  165,  28,  0,  0,  24,  0,  0,  160,  64,  149,  4,  131,  4,  220,  5,  0,  0,  20,  0,  144,  31,  139,  0,  0,  0,  0,  0,  0,  0,  0,  160,  15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  128,  0,  0,  0,  155,  0,  0,  0,  0,  0,  0,  0,  0,  164,  15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  165,  28,  0,  0,  24,  10,  215,  35,  60,  149,  4,  155,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  160,  15,  0,  0,  24,  246,  40,  32,  65,  128,  2,  137,  27,  149,  0,  0,  0,  0,  66,  121,  32,  104,  97,  110,  100,  32,  109,  111,  118,  101,  32,  116,  104,  101,  32,  101,  120,  45,  0,  149,  1,  0,  0,  0,  116,  114,  117,  100,  101,  114,  32,  116,  111,  32,  100,  105,  102,  102,  101,  114,  101,  110,  116,  32,  0,  149,  1,  0,  0,  0,  112,  111,  115,  105,  116,  105,  111,  110,  115,  32,  111,  118,  101,  114,  32,  116,  104,  101,  32,  32,  0,  149,  7,  0,  0,  0,  98,  117,  105,  108,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  46,  46,  46,  46,  0,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  115,  112,  97,  99,  105,  110,  103,  32,  32,  0,  149,  1,  0,  0,  0,  98,  101,  116,  119,  101,  101,  110,  32,  116,  104,  101,  32,  101,  120,  116,  114,  117,  100,  101,  114,  0,  149,  1,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  32,  0,  149,  7,  0,  0,  0,  119,  105,  116,  104,  32,  116,  104,  101,  32,  107,  110,  111,  98,  115,  46,  46,  46,  0,  149,  0,  0,  0,  0,  117,  110,  100,  101,  114,  32,  116,  104,  101,  32,  112,  108,  97,  116,  102,  111,  114,  109,  32,  32,  0,  149,  1,  0,  0,  0,  97,  110,  100,  32,  97,  32,  115,  104,  101,  101,  116,  32,  111,  102,  32,  112,  97,  112,  101,  114,  0,  149,  1,  0,  0,  0,  112,  108,  97,  99,  101,  100,  32,  98,  101,  116,  119,  101,  101,  110,  32,  116,  104,  101,  32,  32,  0,  149,  7,  0,  0,  0,  112,  108,  97,  116,  102,  111,  114,  109,  32,  97,  110,  100,  32,  116,  104,  101,  46,  46,  46,  0,  149,  0,  0,  0,  0,  110,  111,  122,  122,  108,  101,  46,  32,  87,  104,  101,  110,  32,  121,  111,  117,  32,  97,  114,  101,  0,  149,  1,  0,  0,  0,  100,  111,  110,  101,  44,  32,  112,  114,  101,  115,  115,  32,  116,  104,  101,  32,  32,  32,  32,  32,  0,  149,  7,  0,  0,  0,  99,  101,  110,  116,  101,  114,  32,  98,  117,  116,  116,  111,  110,  46,  0,  137,  31 };

#define LEVEL_PLATE_LEN 571

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
const static PROGMEM prog_uchar SPLASH1_MSG[] = "Sailfish Replicator2";
#else
const static PROGMEM prog_uchar SPLASH1_MSG[] = "  Sailfish Rep 2X   ";
#endif
const static PROGMEM prog_uchar SPLASH2_MSG[] = "--------------------";
#else
#warning "*** Compiling without MODEL_x defined ***"
const static PROGMEM prog_uchar SPLASH1_MSG[] = "      Sailfish      ";
const static PROGMEM prog_uchar SPLASH2_MSG[] = "      --------      ";
#endif
const static PROGMEM prog_uchar SPLASH3_MSG[] = "Thing 32084 " DATE_STR;
const static PROGMEM prog_uchar SPLASH4_MSG[] = "Sailfish v" VERSION_STR " r" SVN_VERSION_STR;


#include "locale.hh"

#endif // __MENU__LOCALES__
