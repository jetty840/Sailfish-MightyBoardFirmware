#ifndef __MENU__LOCALES__
#define __MENU__LOCALES__

#include <avr/pgmspace.h>
#include <string.h>

#define FRENCH 1

static PROGMEM prog_uchar ON_MSG[] =      "ON ";
static PROGMEM prog_uchar OFF_MSG[] =     "OFF";

static PROGMEM prog_uchar ON_CELCIUS_MSG[] = "/   C";
static PROGMEM prog_uchar CELCIUS_MSG[] =    "C    ";
static PROGMEM prog_uchar ARROW_MSG[] =      "-->";
static PROGMEM prog_uchar NO_ARROW_MSG[] =   "   ";
static PROGMEM prog_uchar BLANK_CHAR_MSG[] = " ";

static PROGMEM prog_uchar CLEAR_MSG[] =  "                    ";
static PROGMEM prog_uchar BLANKLINE_MSG[] =  "                ";



#ifdef LOCALE_FR
#include "Menu.FR.hh"
#else // Use US ENGLISH as default

#   define LEVEL_PLATE_DUAL static uint8_t LevelPlateDual[] PROGMEM = { 149,  0,  0,  0,  0,  70,  105,  110,  100,  32,  116,  104,  101,  32,  52,  32,  107,  110,  111,  98,  115,  32,  111,  110,  32,  0,  149,  1,  0,  0,  0,  116,  104,  101,  32,  98,  111,  116,  116,  111,  109,  32,  111,  102,  32,  116,  104,  101,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  108,  97,  116,  102,  111,  114,  109,  32,  97,  110,  100,  32,  116,  105,  103,  104,  116,  101,  110,  0,  149,  7,  0,  0,  0,  102,  111,  117,  114,  32,  111,  114,  32,  102,  105,  118,  101,  32,  116,  117,  114,  110,  115,  46,  0,  149,  0,  0,  0,  0,  73,  39,  109,  32,  103,  111,  105,  110,  103,  32,  116,  111,  32,  109,  111,  118,  101,  32,  32,  32,  0,  149,  1,  0,  0,  0,  116,  104,  101,  32,  101,  120,  116,  114,  117,  100,  101,  114,  32,  116,  111,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  118,  97,  114,  105,  111,  117,  115,  32,  112,  111,  115,  105,  116,  105,  111,  110,  115,  32,  32,  32,  0,  149,  7,  0,  0,  0,  102,  111,  114,  32,  97,  100,  106,  117,  115,  116,  109,  101,  110,  116,  46,  0,  149,  0,  0,  0,  0,  73,  110,  32,  101,  97,  99,  104,  32,  112,  111,  115,  105,  116,  105,  111,  110,  44,  32,  32,  32,  0,  149,  1,  0,  0,  0,  119,  101,  32,  119,  105,  108,  108,  32,  110,  101,  101,  100,  32,  116,  111,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  97,  100,  106,  117,  115,  116,  32,  50,  32,  107,  110,  111,  98,  115,  32,  97,  116,  32,  32,  32,  0,  149,  7,  0,  0,  0,  116,  104,  101,  32,  115,  97,  109,  101,  32,  116,  105,  109,  101,  46,  0,  149,  0,  0,  0,  0,  78,  111,  122,  122,  108,  101,  115,  32,  97,  114,  101,  32,  97,  116,  32,  116,  104,  101,  32,  32,  0,  149,  1,  0,  0,  0,  114,  105,  103,  104,  116,  32,  104,  101,  105,  103,  104,  116,  32,  119,  104,  101,  110,  32,  32,  32,  0,  149,  1,  0,  0,  0,  121,  111,  117,  32,  99,  97,  110,  32,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  32,  97,  0,  149,  7,  0,  0,  0,  115,  104,  101,  101,  116,  32,  111,  102,  32,  112,  97,  112,  101,  114,  0,  149,  0,  0,  0,  0,  98,  101,  116,  119,  101,  101,  110,  32,  116,  104,  101,  32,  110,  111,  122,  122,  108,  101,  32,  32,  0,  149,  1,  0,  0,  0,  97,  110,  100,  32,  116,  104,  101,  32,  112,  108,  97,  116,  102,  111,  114,  109,  46,  32,  32,  32,  0,  149,  1,  0,  0,  0,  71,  114,  97,  98,  32,  97,  32,  115,  104,  101,  101,  116,  32,  111,  102,  32,  32,  32,  32,  32,  0,  149,  7,  0,  0,  0,  112,  97,  112,  101,  114,  32,  116,  111,  32,  97,  115,  115,  105,  115,  116,  32,  117,  115,  46,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  87,  41,  4,  0,  24,  131,  4,  220,  5,  0,  0,  20,  0,  140,  229,  55,  0,  0,  148,  27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  229,  55,  0,  0,  148,  27,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  18,  6,  0,  0,  202,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  218,  223,  55,  0,  24,  142,  17,  6,  0,  0,  202,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  102,  114,  111,  110,  116,  32,  116,  119,  111,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  17,  6,  0,  0,  201,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  17,  6,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  81,  129,  40,  0,  24,  142,  18,  6,  0,  0,  122,  26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  98,  97,  99,  107,  32,  116,  119,  111,  32,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  17,  6,  0,  0,  123,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  42,  39,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  207,  249,  31,  0,  24,  142,  42,  39,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  114,  105,  103,  104,  116,  32,  116,  119,  111,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  42,  39,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  248,  228,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  23,  240,  49,  0,  24,  142,  249,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  108,  101,  102,  116,  32,  116,  119,  111,  32,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  249,  228,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  17,  6,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  11,  248,  24,  0,  24,  142,  17,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  67,  104,  101,  99,  107,  32,  116,  104,  97,  116,  32,  112,  97,  112,  101,  114,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  137,  31 };
#   define LEVEL_PLATE_SINGLE static uint8_t LevelPlateSingle[] PROGMEM = { 149,  0,  0,  0,  0,  70,  105,  110,  100,  32,  116,  104,  101,  32,  52,  32,  107,  110,  111,  98,  115,  32,  111,  110,  32,  0,  149,  1,  0,  0,  0,  116,  104,  101,  32,  98,  111,  116,  116,  111,  109,  32,  111,  102,  32,  116,  104,  101,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  108,  97,  116,  102,  111,  114,  109,  32,  97,  110,  100,  32,  116,  105,  103,  104,  116,  101,  110,  0,  149,  7,  0,  0,  0,  102,  111,  117,  114,  32,  111,  114,  32,  102,  105,  118,  101,  32,  116,  117,  114,  110,  115,  46,  0,  149,  0,  0,  0,  0,  73,  39,  109,  32,  103,  111,  105,  110,  103,  32,  116,  111,  32,  109,  111,  118,  101,  32,  32,  32,  0,  149,  1,  0,  0,  0,  116,  104,  101,  32,  101,  120,  116,  114,  117,  100,  101,  114,  32,  116,  111,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  118,  97,  114,  105,  111,  117,  115,  32,  112,  111,  115,  105,  116,  105,  111,  110,  115,  32,  32,  32,  0,  149,  7,  0,  0,  0,  102,  111,  114,  32,  97,  100,  106,  117,  115,  116,  109,  101,  110,  116,  46,  0,  149,  0,  0,  0,  0,  73,  110,  32,  101,  97,  99,  104,  32,  112,  111,  115,  105,  116,  105,  111,  110,  44,  32,  32,  32,  0,  149,  1,  0,  0,  0,  119,  101,  32,  119,  105,  108,  108,  32,  110,  101,  101,  100,  32,  116,  111,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  97,  100,  106,  117,  115,  116,  32,  50,  32,  107,  110,  111,  98,  115,  32,  97,  116,  32,  32,  32,  0,  149,  7,  0,  0,  0,  116,  104,  101,  32,  115,  97,  109,  101,  32,  116,  105,  109,  101,  46,  0,  149,  0,  0,  0,  0,  78,  111,  122,  122,  108,  101,  115,  32,  97,  114,  101,  32,  97,  116,  32,  116,  104,  101,  32,  32,  0,  149,  1,  0,  0,  0,  114,  105,  103,  104,  116,  32,  104,  101,  105,  103,  104,  116,  32,  119,  104,  101,  110,  32,  32,  32,  0,  149,  1,  0,  0,  0,  121,  111,  117,  32,  99,  97,  110,  32,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  32,  97,  0,  149,  7,  0,  0,  0,  115,  104,  101,  101,  116,  32,  111,  102,  32,  112,  97,  112,  101,  114,  0,  149,  0,  0,  0,  0,  98,  101,  116,  119,  101,  101,  110,  32,  116,  104,  101,  32,  110,  111,  122,  122,  108,  101,  32,  32,  0,  149,  1,  0,  0,  0,  97,  110,  100,  32,  116,  104,  101,  32,  112,  108,  97,  116,  102,  111,  114,  109,  46,  32,  32,  32,  0,  149,  1,  0,  0,  0,  71,  114,  97,  98,  32,  97,  32,  115,  104,  101,  101,  116,  32,  111,  102,  32,  32,  32,  32,  32,  0,  149,  7,  0,  0,  0,  112,  97,  112,  101,  114,  32,  116,  111,  32,  97,  115,  115,  105,  115,  116,  32,  117,  115,  46,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  87,  41,  4,  0,  24,  131,  4,  220,  5,  0,  0,  20,  0,  140,  229,  55,  0,  0,  148,  27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  230,  55,  0,  0,  148,  27,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  0,  0,  0,  0,  202,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  42,  13,  59,  0,  24,  142,  0,  0,  0,  0,  201,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  102,  114,  111,  110,  116,  32,  116,  119,  111,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  0,  0,  0,  0,  202,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  0,  0,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  81,  129,  40,  0,  24,  142,  0,  0,  0,  0,  122,  26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  98,  97,  99,  107,  32,  116,  119,  111,  32,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  0,  0,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  198,  36,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  168,  47,  34,  0,  24,  142,  198,  36,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  114,  105,  103,  104,  116,  32,  116,  119,  111,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  198,  36,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  58,  219,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  139,  124,  55,  0,  24,  142,  58,  219,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  65,  100,  106,  117,  115,  116,  32,  116,  104,  101,  32,  108,  101,  102,  116,  32,  116,  119,  111,  32,  0,  149,  1,  0,  0,  0,  107,  110,  111,  98,  115,  32,  117,  110,  116,  105,  108,  32,  112,  97,  112,  101,  114,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  149,  2,  0,  0,  0,  80,  108,  101,  97,  115,  101,  32,  119,  97,  105,  116,  0,  142,  58,  219,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  142,  0,  0,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  69,  190,  27,  0,  24,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  13,  69,  3,  0,  24,  149,  0,  0,  0,  0,  67,  104,  101,  99,  107,  32,  116,  104,  97,  116,  32,  112,  97,  112,  101,  114,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  106,  117,  115,  116,  32,  115,  108,  105,  100,  101,  115,  32,  98,  101,  116,  119,  101,  101,  110,  32,  0,  149,  7,  0,  0,  0,  110,  111,  122,  122,  108,  101,  32,  97,  110,  100,  32,  112,  108,  97,  116,  102,  111,  114,  109,  0,  137,  31 };
#   define LEVEL_PLATE_LEN 1576


static PROGMEM prog_uchar SPLASH1_MSG[] = "  The Replicator    ";
static PROGMEM prog_uchar SPLASH2_MSG[] = "    ----------      ";
static PROGMEM prog_uchar SPLASH3_MSG[] = "                    ";
static PROGMEM prog_uchar SPLASH4_MSG[] = "Firmware Version 5. ";

static PROGMEM prog_uchar SPLASH1A_MSG[] = "    FAIL!           ";
static PROGMEM prog_uchar SPLASH2A_MSG[] = "    SUCCESS!        ";
static PROGMEM prog_uchar SPLASH3A_MSG[] = "connected correctly ";
static PROGMEM prog_uchar SPLASH4A_MSG[] = "Heaters are not     ";
static PROGMEM prog_uchar SPLASH5A_MSG[] = "                    ";	

static PROGMEM prog_uchar GO_MSG[] =         "Start Preheat!";
static PROGMEM prog_uchar STOP_MSG[] =       "Cool!";
static PROGMEM prog_uchar RIGHT_TOOL_MSG[] = "Right Tool";
static PROGMEM prog_uchar LEFT_TOOL_MSG[] =  "Left Tool";
static PROGMEM prog_uchar PLATFORM_MSG[] =   "Platform";
static PROGMEM prog_uchar TOOL_MSG[] =       "Extruder";

static PROGMEM prog_uchar START_MSG[] =      "Welcome!            " "I'm the Replicator. " "Press the red M to  " "get started!        ";
static PROGMEM prog_uchar BUTTONS1_MSG[] =   "A blinking 'M' means" "I'm waiting and will" "continue when you   " "press the button... ";
static PROGMEM prog_uchar BUTTONS2_MSG[] =   "A solid 'M' means   " "I'm working and will" "update my status    " "when I'm finished...";
static PROGMEM prog_uchar EXPLAIN_MSG[] =    "Our next steps will " "get me set up to    " "print! First, we'll " "restore my build... ";                                  
static PROGMEM prog_uchar LEVEL_MSG[] =      "platform so it's    " "nice and level. It's" "probably a bit off  " "from shipping...    ";
static PROGMEM prog_uchar BETTER_MSG[] =     "Aaah, that feels    " "much better.        " "Let's go on and load" "some plastic!       ";
static PROGMEM prog_uchar TRYAGAIN_MSG[] =   "We'll try again!    " "                    " "                    " "                    ";
static PROGMEM prog_uchar GO_ON_MSG[]   =    "We'll keep going    " "and load some       " "plastic! For help go" "to makerbot.com/help";     
static PROGMEM prog_uchar SD_MENU_MSG[] =    "Awesome!            " "We'll go to the SD  " "card Menu and you   " "can select a print! ";
static PROGMEM prog_uchar FAIL_MSG[] =       "We'll go to the main" "menu. If you need   " "help go to:         " "makerbot.com/help   ";
static PROGMEM prog_uchar START_TEST_MSG[]=  "I'm going to print  " "a series of lines so" "we can find my      " "nozzle alignment.   "; // XXX old name: start[]
static PROGMEM prog_uchar EXPLAIN1_MSG[] =   "Look for the best   " "matched line in each" "axis set. Lines are " "numbered 1-13 and...";
static PROGMEM prog_uchar EXPLAIN2_MSG[] =   "line one is extra   " "long. The Y axis set" "is left on the plate" "and X axis is right.";
static PROGMEM prog_uchar END_MSG  [] =      "Great!  I've saved  " "these settings and  " "I'll use them to    " "make nice prints!   ";

static PROGMEM prog_uchar SELECT_MSG[] =     "Select best lines.";
static PROGMEM prog_uchar DONE_MSG[]   =     "Done!";
static PROGMEM prog_uchar NO_MSG[]   =       "No";
static PROGMEM prog_uchar YES_MSG[]  =       "Yes";

static PROGMEM prog_uchar XAXIS_MSG[] =      "X Axis Line";
static PROGMEM prog_uchar YAXIS_MSG[] =      "Y Axis Line";

static PROGMEM prog_uchar HEATER_ERROR_MSG[]=  "My extruders are    " "not heating up.     " "Check my            " "connections!        ";
static PROGMEM prog_uchar EXPLAIN_ONE_MSG[] =  "Press down on the   " "grey rings at top of" "the extruders and   " "pull the black...   ";
static PROGMEM prog_uchar EXPLAIN_TWO_MSG[] =  "guide tubes out. Now" "feed filament from  " "the back through the" "tubes until it...   ";
static PROGMEM prog_uchar EXPLAIN_THRE_MSG[]=  "pops out in front.  " "I'm heating up my   " "extruder so we can  " "load the filament...";
static PROGMEM prog_uchar EXPLAIN_FOUR_MSG[]=  "This might take a   " "few minutes.        " "And watch out, the  " "nozzle will get HOT!";
static PROGMEM prog_uchar EXPLAIN_ONE_S_MSG[]= "Press down on the   " "grey ring at top of " "the extruder and    " "pull the black...   ";
static PROGMEM prog_uchar EXPLAIN_TWO_S_MSG[]= "guide tube out.  Now" "feed filament from  " "the back through the" "tube until it...   ";

static PROGMEM prog_uchar HEATING_BAR_MSG[] = "I'm heating up my   " "extruder!           " "Please wait.        " "                    ";
static PROGMEM prog_uchar HEATING_PROG_MSG[]= "Heating Progress:   " "                    " "                    " "                    ";
static PROGMEM prog_uchar READY_RIGHT_MSG[] = "OK I'm ready!       " "First we'll load the" "right extruder.     " "Push filament in... ";
static PROGMEM prog_uchar READY_SINGLE_MSG[]= "OK I'm ready!       " "Pop the guide tube  " "off and push the    " "filament down...    "; 
static PROGMEM prog_uchar READY_REV_MSG[]   = "OK I'm ready!       " "Pop the guide tube  " "off and pull        " "filament gently...  ";
static PROGMEM prog_uchar READY_LEFT_MSG[]  = "Great! Now we'll    " "load the left       " "extruder. Push      " "filament down...    ";
static PROGMEM prog_uchar TUG_MSG[]         = "through the grey    " "ring until you feel " "the motor tugging   " "the plastic in...   ";
static PROGMEM prog_uchar STOP_MSG_MSG[]    = "When filament is    " "extruding out of the" "nozzle, Press 'M'   " "to stop extruding.  ";  // XXX old name: stop[]
static PROGMEM prog_uchar STOP_EXIT_MSG[]   = "When filament is    " "extruding out of the" "nozzle, Press 'M'   " "to exit             "; 
static PROGMEM prog_uchar STOP_REVERSE_MSG[]= "When my filament is " "released,           " "Press 'M' to exit.  " "                    ";            
static PROGMEM prog_uchar PUSH_HARDER_MSG[] = "OK! I'll keep my    " "motor running. You  " "may need to push    " "harder...           ";  // XXX old name: tryagain[]
static PROGMEM prog_uchar KEEP_GOING_MSG[]  = "We'll keep going.   " "If you're having    " "trouble, check out  " "makerbot.com/help   ";  // XXX old name: go_on[]
static PROGMEM prog_uchar FINISH_MSG[]      = "Great!  I'll stop   " "running my extruder." "Press M to continue." "                    ";  
static PROGMEM prog_uchar GO_ON_LEFT_MSG[]  = "We'll keep going.   " "Lets try the left   " "extruder. Push      " "filament down...    ";
static PROGMEM prog_uchar TIMEOUT_MSG[]		= "My motor timed out  " "after 5 minutes.    " "Press M to exit.    " "                    ";

static PROGMEM prog_uchar READY1_MSG[] =     "How'd it go? Ready ";
static PROGMEM prog_uchar READY2_MSG[] =     "to try a print?    ";
static PROGMEM prog_uchar NOZZLE_MSG_MSG[] = "Does my nozzle";        // XXX old name: ready1[]
static PROGMEM prog_uchar HEIGHT_CHK_MSG[] = "height check out?";     // XXX old name: ready2[]
static PROGMEM prog_uchar HEIGHT_GOOD_MSG[]= "My height is good!";   // XXX old name: yes[]
static PROGMEM prog_uchar TRY_AGAIN_MSG[]  = "Let's try again.";       // XXX old name: no[]

static PROGMEM prog_uchar QONE_MSG[] =       "Did plastic extrude ";
static PROGMEM prog_uchar QTWO_MSG[] =       "from the nozzle?";
static PROGMEM prog_uchar LOAD_RIGHT_MSG[] = "Load right"; 
static PROGMEM prog_uchar LOAD_LEFT_MSG[] =  "Load left"; 
static PROGMEM prog_uchar LOAD_SINGLE_MSG[] ="Load"; 
static PROGMEM prog_uchar UNLOAD_SINGLE_MSG[]="Unload"; 
static PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]="Unload right"; 
static PROGMEM prog_uchar UNLOAD_LEFT_MSG[] ="Unload left"; 

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


static PROGMEM prog_uchar DISTANCESHORT_MSG[] = "SHORT";
static PROGMEM prog_uchar DISTANCELONG_MSG[] =  "LONG";
static PROGMEM prog_uchar GAMEOVER_MSG[] =      "GAME OVER!";


static PROGMEM prog_uchar HEATING_MSG[] =        "Heating:";
static PROGMEM prog_uchar HEATING_SPACES_MSG[] = "Heating:            ";

static PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
static PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "R Extruder: ---/---C";
static PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "L Extruder: ---/---C";
static PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Platform:   ---/---C";
static PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Extruder:   ---/---C";


static PROGMEM prog_uchar PREHEAT_SET_MSG[] = "Preheat Settings    ";
static PROGMEM prog_uchar RIGHT_SPACES_MSG[] = "Right Tool          ";    // XXX old name: right[]
static PROGMEM prog_uchar LEFT_SPACES_MSG[]   = "Left Tool           ";   // XXX old name: left[]
static PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Platform        ";    // XXX old name: platform[]
static PROGMEM prog_uchar RESET1_MSG[] = "Reset Settings to ";       // XXX old name: set1[]
static PROGMEM prog_uchar RESET2_MSG[] = "Default values?";
static PROGMEM prog_uchar CANCEL_MSG[] = "Cancel this print?";
static PROGMEM prog_uchar CANCEL_PROCESS_MSG[] = "Quit this process?";

static PROGMEM prog_uchar PAUSE_MSG[] = "Pause";
static PROGMEM prog_uchar UNPAUSE_MSG[] = "UnPause";


static PROGMEM prog_uchar NOCARD_MSG[] = "No SD card found";

static PROGMEM prog_uchar BUILD_MSG[] =            "Print from SD";
static PROGMEM prog_uchar PREHEAT_MSG[] =          "Preheat";
static PROGMEM prog_uchar UTILITIES_MSG[] =        "Utilities";
static PROGMEM prog_uchar MONITOR_MSG[] =          "Monitor Mode";
static PROGMEM prog_uchar JOG_MSG[]   =            "Jog Mode";
static PROGMEM prog_uchar CALIBRATION_MSG[] =      "Calibrate Axes";
static PROGMEM prog_uchar HOME_AXES_MSG[] =        "Home Axes";
static PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Filament Options";
static PROGMEM prog_uchar STARTUP_MSG[] =          "Run Startup Script";
static PROGMEM prog_uchar VERSION_MSG[] = 		   "Version Number";
static PROGMEM prog_uchar DSTEPS_MSG[] =           "Disable Steppers";
static PROGMEM prog_uchar ESTEPS_MSG[] =           "Enable Steppers  ";
static PROGMEM prog_uchar PLATE_LEVEL_MSG[] =      "Level Build Plate";
static PROGMEM prog_uchar LED_BLINK_MSG[] =        "Blink LEDs       ";
static PROGMEM prog_uchar LED_STOP_MSG[] =         "Stop Blinking!";
static PROGMEM prog_uchar PREHEAT_SETTINGS_MSG[] = "Preheat Settings";
static PROGMEM prog_uchar SETTINGS_MSG[] =         "General Settings";
static PROGMEM prog_uchar RESET_MSG[] =            "Restore Defaults";
static PROGMEM prog_uchar NOZZLES_MSG[] =          "Calibrate Nozzles";
static PROGMEM prog_uchar TOOL_COUNT_MSG[]   =     "Tool Count ";
static PROGMEM prog_uchar SOUND_MSG[] =            "Sound";
static PROGMEM prog_uchar LED_MSG[] =              "LED Color     ";
static PROGMEM prog_uchar LED_HEAT_MSG[] =         "Heat LEDs";
static PROGMEM prog_uchar HELP_SCREENS_MSG[] =     "Help Text  ";
static PROGMEM prog_uchar EXIT_MSG[] =             "exit menu";
static PROGMEM prog_uchar ACCELERATE_MSG[] = 	   "Accelerate";

static PROGMEM prog_uchar RED_COLOR_MSG[]    = "RED   ";
static PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE ";
static PROGMEM prog_uchar PINK_COLOR_MSG[]   = "PINK  ";
static PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "GREEN ";
static PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLUE  ";
static PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "PURPLE";
static PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "WHITE ";
static PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "CUSTOM";

static PROGMEM prog_uchar TOOL_SINGLE_MSG[] = "SINGLE";
static PROGMEM prog_uchar TOOL_DUAL_MSG[] =   "DUAL  ";

static PROGMEM prog_uchar RIGHT_MSG[] =   "Right";
static PROGMEM prog_uchar LEFT_MSG[] =    "Left";
static PROGMEM prog_uchar ERROR_MSG[] =   "error!";
static PROGMEM prog_uchar NA_MSG[] =      "  NA    ";
static PROGMEM prog_uchar WAITING_MSG[] = "waiting ";

#define HEATER_INACTIVITY_MSG           "Heaters shutdown    " "due to inactivity   "
#define HEATER_FAIL_SOFTWARE_CUTOFF_MSG "Extruder Overheat!  " "Software Temp Limit " "Reached! Please     " "Shutdown or Restart"
#define HEATER_FAIL_HARDWARE_CUTOFF_MSG "Extruder Overheat!  " "Safety Cutoff       " "Triggered! Please   " "Shutdown or Restart"
#define HEATER_FAIL_NOT_HEATING_MSG     "Heating Failure!    " "My extruders are not" "heating properly.   " "Check my connections"
#define HEATER_FAIL_DROPPING_TEMP_MSG   "Heating Failure!    " "My extruders are    " "losing temperature. " "Check my connections"
#define HEATER_FAIL_NOT_PLUGGED_IN_MSG  "Heater Error!       " "My temperature reads" "are failing! Please " "Check my connections"
#endif //end of default ELSE for US English */



#endif // __MENU__LOCALES__
