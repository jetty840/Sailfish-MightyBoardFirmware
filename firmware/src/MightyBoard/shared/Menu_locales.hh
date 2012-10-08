#ifndef __MENU__LOCALES__
#define __MENU__LOCALES__

#include <avr/pgmspace.h>
#include <string.h>

#define FRENCH 1

const static PROGMEM prog_uchar ON_MSG[] =      "ON ";
const static PROGMEM prog_uchar OFF_MSG[] =     "OFF";

const static PROGMEM prog_uchar ON_CELCIUS_MSG[] = "/   C";
const static PROGMEM prog_uchar CELCIUS_MSG[] =    "C    ";
const static PROGMEM prog_uchar ARROW_MSG[] =      "-->";
const static PROGMEM prog_uchar NO_ARROW_MSG[] =   "   ";
const static PROGMEM prog_uchar BLANK_CHAR_MSG[] = " ";
const static PROGMEM prog_uchar BLANK_CHAR_4_MSG[] = "    ";

const static PROGMEM prog_uchar CLEAR_MSG[] =  "                    ";
const static PROGMEM prog_uchar BLANKLINE_MSG[] =  "                ";



#ifdef LOCALE_FR
#include "Menu.FR.hh"
#else // Use US ENGLISH as default

#   define LEVEL_PLATE_DUAL const static uint8_t LevelPlateDual[] PROGMEM = { 149,  0,  0,  0,  0,  70,  105,  110,  100,  32,  116,  104,  101,  32,  52,  32,  107,  110,  111,  98,  115,  32,  111,  110,  32,  0,  149,  1,  0,  0,  0,  116,  104,  101,  32,  98,  111,  116,  116,  111,  109,  32,  111,  102,  32,  116,  104,  101,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  108,  97,  116,  102,  111,  114,  109,  32,  97,  110,  100,  32,  116,  105,  103,  104,  116,  101,  110,  0,  149,  7,  0,  0,  0,  102,  111,  117,  114,  32,  111,  114,  32,  102,  105,  118,  101,  32,  116,  117,  114,  110,  115,  46,  0,  149,  0,  0,  0,  0,  73,  39,  109,  32,  103,  111,  105,  110,  103,  32,  116,  111,  32,  109,  111,  118,  101,  32,  32,  32,  0,  149,  1,  0,  0,  0,  116,  104,  101,  32,  101,  120,  116,  114,  117,  100,  101,  114,  32,  116,  111,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  118,  97,  114,  105,  111,  117,  115,  32,  112,  111,  115,  105,  116,  105,  111,  110,  115,  32,  32,  32,  0,  149,  7,  0,  0,  0,  102,  111,  114,  32,  97,  100,  106,  117,  115,  116,  109,  101,  110,  116,  46,  0,  149,  0,  0,  0,  0,  73,  110,  32,  101,  97,  99,  104,  32,  112,  111,  115,  105,  116,  105,  111,  110,  44,  32,  32,  32,  0,  149,  1,  0,  0,  0,  119,  101,  32,  119,  105,  108,  108,  32,  110,  101,  101,  100,  32,  116,  111,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  97,  100,  106,  117,  115,  116,  32,  50,  32,  107,  110,  111,  98,  115,  32,  97,  116,  32,  32,  32,  0,  149,  7,  0,  0,  0,  116,  104,  101,  32,  115,  97,  109,  101,  32,  116,  105,  109,  101,  46,  0,  149,  0,  0,  0,  0,  78,  111,  122,  122,  108,  101,  115,  32,  97,  114,  101,  32,  97,  116,  32,  116,  104,  101,  32,  32,  0,  149,  1,  0,  0,  0,  114,  105,  103,  104,  116,  32,  104,  101,  105,  103,  104,  116,  32,  119,  104,  101,  110,  32,  32,  32,  0,  149,  1,  0,  0,  0,  121,  111,  117,  32,  99,  97,  110,  32,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  32,  97,  0,  149,  7,  0,  0,  0,  115,  104,  101,  101,  116,  32,  111,  102,  32,  112,  97,  112,  101,  114,  0,  149,  0,  0,  0,  0,  98,  101,  116,  119,  101,  101,  110,  32,  116,  104,  101,  32,  110,  111,  122,  122,  108,  101,  32,  32,  0,  149,  1,  0,  0,  0,  97,  110,  100,  32,  116,  104,  101,  32,  112,  108,  97,  116,  102,  111,  114,  109,  46,  32,  32,  32,  0,  149,  1,  0,  0,  0,  71,  114,  97,  98,  32,  97,  32,  115,  104,  101,  101,  116,  32,  111,  102,  32,  32,  32,  32,  32,  0,  149,  7,  0,  0,  0,  112,  97,  112,  101,  114,  32,  116,  111,  32,  97,  115,  115,  105,  115,  116,  32,  117,  115,  46,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  87,  41,  4,  0,  24,  131,  4,  220,  5,  0,  0,  20,  0,  140,  229,  55,  0,  0,  148,  27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  229,  55,  0,  0,  148,  27,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  18,  6,  0,  0,  202,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  218,  223,  55,  0,  24,  142,  17,  6,  0,  0,  202,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  102,  114,  111,  110,  116,  32,  116,  119,  111,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  17,  6,  0,  0,  201,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  17,  6,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  81,  129,  40,  0,  24,  142,  18,  6,  0,  0,  122,  26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  98,  97,  99,  107,  32,  116,  119,  111,  32,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  17,  6,  0,  0,  123,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  42,  39,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  207,  249,  31,  0,  24,  142,  42,  39,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  114,  105,  103,  104,  116,  32,  116,  119,  111,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  42,  39,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  248,  228,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  23,  240,  49,  0,  24,  142,  249,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  108,  101,  102,  116,  32,  116,  119,  111,  32,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  249,  228,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  17,  6,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  11,  248,  24,  0,  24,  142,  17,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  67,  104,  101,  99,  107,  32,  116,  104,  97,  116,  32,  112,  97,  112,  101,  114,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  137,  31 };
#   define LEVEL_PLATE_SINGLE const static uint8_t LevelPlateSingle[] PROGMEM = { 149,  0,  0,  0,  0,  70,  105,  110,  100,  32,  116,  104,  101,  32,  52,  32,  107,  110,  111,  98,  115,  32,  111,  110,  32,  0,  149,  1,  0,  0,  0,  116,  104,  101,  32,  98,  111,  116,  116,  111,  109,  32,  111,  102,  32,  116,  104,  101,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  108,  97,  116,  102,  111,  114,  109,  32,  97,  110,  100,  32,  116,  105,  103,  104,  116,  101,  110,  0,  149,  7,  0,  0,  0,  102,  111,  117,  114,  32,  111,  114,  32,  102,  105,  118,  101,  32,  116,  117,  114,  110,  115,  46,  0,  149,  0,  0,  0,  0,  73,  39,  109,  32,  103,  111,  105,  110,  103,  32,  116,  111,  32,  109,  111,  118,  101,  32,  32,  32,  0,  149,  1,  0,  0,  0,  116,  104,  101,  32,  101,  120,  116,  114,  117,  100,  101,  114,  32,  116,  111,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  118,  97,  114,  105,  111,  117,  115,  32,  112,  111,  115,  105,  116,  105,  111,  110,  115,  32,  32,  32,  0,  149,  7,  0,  0,  0,  102,  111,  114,  32,  97,  100,  106,  117,  115,  116,  109,  101,  110,  116,  46,  0,  149,  0,  0,  0,  0,  73,  110,  32,  101,  97,  99,  104,  32,  112,  111,  115,  105,  116,  105,  111,  110,  44,  32,  32,  32,  0,  149,  1,  0,  0,  0,  119,  101,  32,  119,  105,  108,  108,  32,  110,  101,  101,  100,  32,  116,  111,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  97,  100,  106,  117,  115,  116,  32,  50,  32,  107,  110,  111,  98,  115,  32,  97,  116,  32,  32,  32,  0,  149,  7,  0,  0,  0,  116,  104,  101,  32,  115,  97,  109,  101,  32,  116,  105,  109,  101,  46,  0,  149,  0,  0,  0,  0,  78,  111,  122,  122,  108,  101,  115,  32,  97,  114,  101,  32,  97,  116,  32,  116,  104,  101,  32,  32,  0,  149,  1,  0,  0,  0,  114,  105,  103,  104,  116,  32,  104,  101,  105,  103,  104,  116,  32,  119,  104,  101,  110,  32,  32,  32,  0,  149,  1,  0,  0,  0,  121,  111,  117,  32,  99,  97,  110,  32,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  32,  97,  0,  149,  7,  0,  0,  0,  115,  104,  101,  101,  116,  32,  111,  102,  32,  112,  97,  112,  101,  114,  0,  149,  0,  0,  0,  0,  98,  101,  116,  119,  101,  101,  110,  32,  116,  104,  101,  32,  110,  111,  122,  122,  108,  101,  32,  32,  0,  149,  1,  0,  0,  0,  97,  110,  100,  32,  116,  104,  101,  32,  112,  108,  97,  116,  102,  111,  114,  109,  46,  32,  32,  32,  0,  149,  1,  0,  0,  0,  71,  114,  97,  98,  32,  97,  32,  115,  104,  101,  101,  116,  32,  111,  102,  32,  32,  32,  32,  32,  0,  149,  7,  0,  0,  0,  112,  97,  112,  101,  114,  32,  116,  111,  32,  97,  115,  115,  105,  115,  116,  32,  117,  115,  46,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  87,  41,  4,  0,  24,  131,  4,  220,  5,  0,  0,  20,  0,  140,  229,  55,  0,  0,  148,  27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  230,  55,  0,  0,  148,  27,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  0,  0,  0,  0,  202,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  42,  13,  59,  0,  24,  142,  0,  0,  0,  0,  201,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  102,  114,  111,  110,  116,  32,  116,  119,  111,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  0,  0,  0,  0,  202,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  0,  0,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  81,  129,  40,  0,  24,  142,  0,  0,  0,  0,  122,  26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  98,  97,  99,  107,  32,  116,  119,  111,  32,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  0,  0,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  198,  36,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  168,  47,  34,  0,  24,  142,  198,  36,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  114,  105,  103,  104,  116,  32,  116,  119,  111,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  198,  36,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  58,  219,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  139,  124,  55,  0,  24,  142,  58,  219,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  108,  101,  102,  116,  32,  116,  119,  111,  32,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  58,  219,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  0,  0,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  69,  190,  27,  0,  24,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  67,  104,  101,  99,  107,  32,  116,  104,  97,  116,  32,  112,  97,  112,  101,  114,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  137,  31 };
#   define LEVEL_PLATE_LEN 1576


const static PROGMEM prog_uchar SPLASH1_MSG[] = "  The Replicator    ";
const static PROGMEM prog_uchar SPLASH2_MSG[] = "    ----------      ";
const static PROGMEM prog_uchar SPLASH3_MSG[] = "                    ";
const static PROGMEM prog_uchar SPLASH4_MSG[] = "Firmware Version 5. ";
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

const static PROGMEM prog_uchar START_MSG[] =      "Welcome!            " "I'm the Replicator. " "Press the red M to  " "get started!        ";
const static PROGMEM prog_uchar BUTTONS1_MSG[] =   "A blinking 'M' means" "I'm waiting and will" "continue when you   " "press the button... ";
const static PROGMEM prog_uchar BUTTONS2_MSG[] =   "A solid 'M' means   " "I'm working and will" "update my status    " "when I'm finished...";
const static PROGMEM prog_uchar EXPLAIN_MSG[] =    "Our next steps will " "get me set up to    " "print! First, we'll " "restore my build... ";                                  
const static PROGMEM prog_uchar LEVEL_MSG[] =      "platform so it's    " "nice and level. It's" "probably a bit off  " "from shipping...    ";
const static PROGMEM prog_uchar BETTER_MSG[] =     "Aaah, that feels    " "much better.        " "Let's go on and load" "some plastic!       ";
const static PROGMEM prog_uchar TRYAGAIN_MSG[] =   "We'll try again!    " "                    " "                    " "                    ";
const static PROGMEM prog_uchar GO_ON_MSG[]   =    "We'll keep going    " "and load some       " "plastic! For help go" "to makerbot.com/help";     
const static PROGMEM prog_uchar SD_MENU_MSG[] =    "Awesome!            " "We'll go to the SD  " "card Menu and you   " "can select a print! ";
const static PROGMEM prog_uchar FAIL_MSG[] =       "We'll go to the main" "menu. If you need   " "help go to:         " "makerbot.com/help   ";
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
const static PROGMEM prog_uchar EXPLAIN_ONE_MSG[] =  "Press down on the   " "grey rings at top of" "the extruders and   " "pull the black...   ";
const static PROGMEM prog_uchar EXPLAIN_TWO_MSG[] =  "guide tubes out. Now" "feed filament from  " "the back through the" "tubes until it...   ";
const static PROGMEM prog_uchar EXPLAIN_THRE_MSG[]=  "pops out in front.  " "I'm heating up my   " "extruder so we can  " "load the filament...";
const static PROGMEM prog_uchar EXPLAIN_FOUR_MSG[]=  "This might take a   " "few minutes.        " "And watch out, the  " "nozzle will get HOT!";
const static PROGMEM prog_uchar EXPLAIN_ONE_S_MSG[]= "Press down on the   " "grey ring at top of " "the extruder and    " "pull the black...   ";
const static PROGMEM prog_uchar EXPLAIN_TWO_S_MSG[]= "guide tube out.  Now" "feed filament from  " "the back through the" "tube until it...   ";

const static PROGMEM prog_uchar HEATING_BAR_MSG[] = "I'm heating up my   " "extruder!           " "Please wait.        " "                    ";
const static PROGMEM prog_uchar HEATING_PROG_MSG[]= "Heating Progress:   " "                    " "                    " "                    ";
const static PROGMEM prog_uchar READY_RIGHT_MSG[] = "OK I'm ready!       " "First we'll load the" "right extruder.     " "Push filament in... ";
const static PROGMEM prog_uchar READY_SINGLE_MSG[]= "OK I'm ready!       " "Pop the guide tube  " "off and push the    " "filament down...    "; 
const static PROGMEM prog_uchar READY_REV_MSG[]   = "OK I'm ready!       " "Pop the guide tube  " "off and pull        " "filament gently...  ";
const static PROGMEM prog_uchar READY_LEFT_MSG[]  = "Great! Now we'll    " "load the left       " "extruder. Push      " "filament down...    ";
const static PROGMEM prog_uchar TUG_MSG[]         = "through the grey    " "ring until you feel " "the motor tugging   " "the plastic in...   ";
const static PROGMEM prog_uchar STOP_MSG_MSG[]    = "When filament is    " "extruding out of the" "nozzle, Press 'M'   " "to stop extruding.  ";  // XXX old name: stop[]
const static PROGMEM prog_uchar STOP_EXIT_MSG[]   = "When filament is    " "extruding out of the" "nozzle, Press 'M'   " "to exit             "; 
const static PROGMEM prog_uchar STOP_REVERSE_MSG[]= "When my filament is " "released,           " "Press 'M' to exit.  " "                    ";            
const static PROGMEM prog_uchar PUSH_HARDER_MSG[] = "OK! I'll keep my    " "motor running. You  " "may need to push    " "harder...           ";  // XXX old name: tryagain[]
const static PROGMEM prog_uchar KEEP_GOING_MSG[]  = "We'll keep going.   " "If you're having    " "trouble, check out  " "makerbot.com/help   ";  // XXX old name: go_on[]
const static PROGMEM prog_uchar FINISH_MSG[]      = "Great!  I'll stop   " "running my extruder." "Press M to continue." "                    ";  
const static PROGMEM prog_uchar GO_ON_LEFT_MSG[]  = "We'll keep going.   " "Lets try the left   " "extruder. Push      " "filament down...    ";
const static PROGMEM prog_uchar TIMEOUT_MSG[]		= "My motor timed out  " "after 5 minutes.    " "Press M to exit.    " "                    ";

const static PROGMEM prog_uchar READY1_MSG[] =     "How'd it go? Ready ";
const static PROGMEM prog_uchar READY2_MSG[] =     "to try a print?    ";
const static PROGMEM prog_uchar NOZZLE_MSG_MSG[] = "Does my nozzle";        // XXX old name: ready1[]
const static PROGMEM prog_uchar HEIGHT_CHK_MSG[] = "height check out?";     // XXX old name: ready2[]
const static PROGMEM prog_uchar HEIGHT_GOOD_MSG[]= "My height is good!";   // XXX old name: yes[]
const static PROGMEM prog_uchar TRY_AGAIN_MSG[]  = "Let's try again.";       // XXX old name: no[]

const static PROGMEM prog_uchar QONE_MSG[] =       "Did plastic extrude ";
const static PROGMEM prog_uchar QTWO_MSG[] =       "from the nozzle?";
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
const static PROGMEM prog_uchar GAMEOVER_MSG[] =      "GAME OVER!";


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
const static PROGMEM prog_uchar CARDREMOVED_MSG[] = "SD Card Removed";
const static PROGMEM prog_uchar NOCARD_MSG[] = "No SD card found";
const static PROGMEM prog_uchar CARDERROR_MSG[] = "SD card read error";
const static PROGMEM prog_uchar CARDERROR2_MSG[] = " SD card read error";
const static PROGMEM prog_uchar FTOOLONG_MSG[] = " Filename too long!";
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
const static PROGMEM prog_uchar STARTUP_MSG[] =          "Run Startup Script";
const static PROGMEM prog_uchar VERSION_MSG[] = 		   "Version Number";
const static PROGMEM prog_uchar DSTEPS_MSG[] =           "Disable Steppers";
const static PROGMEM prog_uchar ESTEPS_MSG[] =           "Enable Steppers  ";
const static PROGMEM prog_uchar PLATE_LEVEL_MSG[] =      "Level Build Plate";
const static PROGMEM prog_uchar LED_BLINK_MSG[] =        "Blink LEDs       ";
const static PROGMEM prog_uchar LED_STOP_MSG[] =         "Stop Blinking!";
const static PROGMEM prog_uchar PREHEAT_SETTINGS_MSG[] = "Preheat Settings";
const static PROGMEM prog_uchar SETTINGS_MSG[] =         "General Settings";
const static PROGMEM prog_uchar RESET_MSG[] =            "Restore Defaults";
const static PROGMEM prog_uchar NOZZLES_MSG[] =          "Calibrate Nozzles";
const static PROGMEM prog_uchar TOOL_COUNT_MSG[]   =     "Tool Count ";
const static PROGMEM prog_uchar SOUND_MSG[] =            "Sound";
const static PROGMEM prog_uchar LED_MSG[] =              "LED Color     ";
const static PROGMEM prog_uchar LED_HEAT_MSG[] =         "Heat LEDs";
const static PROGMEM prog_uchar HELP_SCREENS_MSG[] =     "Help Text  ";
const static PROGMEM prog_uchar EXIT_MSG[] =             "exit menu";
const static PROGMEM prog_uchar ACCELERATE_MSG[] = 	   "Accelerate";
const static PROGMEM prog_uchar OVERRIDE_GCODE_TEMP_MSG[]="OverrideGT";
const static PROGMEM prog_uchar PAUSE_HEAT_MSG[]	 ="Pause Heat";
#ifdef DITTO_PRINT
	const static PROGMEM prog_uchar DITTO_PRINT_MSG[]="DittoPrint";
#endif
const static PROGMEM prog_uchar PAUSEATZPOS_MSG[]	 ="Pause at ZPos";

const static PROGMEM prog_uchar RED_COLOR_MSG[]    = "RED   ";
const static PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE ";
const static PROGMEM prog_uchar PINK_COLOR_MSG[]   = "PINK  ";
const static PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "GREEN ";
const static PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLUE  ";
const static PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "PURPLE";
const static PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "WHITE ";
const static PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "CUSTOM";

const static PROGMEM prog_uchar TOOL_SINGLE_MSG[] = "SINGLE";
const static PROGMEM prog_uchar TOOL_DUAL_MSG[] =   "DUAL  ";

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

const static PROGMEM prog_uchar PRINTED_TOO_LONG_MSG[]		= "Printed too long";

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

#endif //end of default ELSE for US English */



#endif // __MENU__LOCALES__
