
#ifndef FRENCH
#	error no french local defined!
#endif

// XXX REALLY UGLY HACK
// to change me, take the gcode source in s3g scripts directory of firmware/src
// change the strings inside it
// compile the gcode into s3g using replicatorg (or any kind of cli tool)
// run loadDataFile.py to get a byte array
#   define LEVEL_PLATE_DUAL static uint8_t LevelPlateDual[] PROGMEM = { 149,  0,  0,  0,  0,  84,  111,  117,  114,  110,  101,  122,  32,  108,  101,  115,  32,  52,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  115,  111,  117,  115,  32,  108,  97,  32,  112,  108,  97,  113,  117,  101,  32,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  99,  104,  97,  117,  102,  102,  97,  110,  116,  101,  32,  101,  116,  32,  115,  101,  114,  114,  101,  122,  0,  149,  7,  0,  0,  0,  100,  101,  32,  52,  32,  111,  117,  32,  53,  32,  116,  111,  117,  114,  115,  46,  0,  149,  0,  0,  0,  0,  79,  110,  32,  118,  97,  32,  97,  106,  117,  115,  116,  101,  114,  32,  108,  101,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  116,  101,  116,  101,  115,  44,  32,  106,  101,  32,  118,  97,  105,  115,  32,  100,  111,  110,  99,  32,  0,  149,  1,  0,  0,  0,  108,  101,  115,  32,  100,  101,  112,  108,  97,  99,  101,  114,  32,  100,  97,  110,  115,  32,  32,  32,  0,  149,  7,  0,  0,  0,  100,  105,  102,  102,  101,  114,  101,  110,  116,  101,  115,  32,  112,  111,  115,  105,  116,  46,  0,  149,  0,  0,  0,  0,  80,  111,  117,  114,  32,  99,  104,  97,  113,  117,  101,  32,  112,  111,  115,  105,  116,  44,  32,  32,  0,  149,  1,  0,  0,  0,  110,  111,  117,  115,  32,  97,  108,  108,  111,  110,  115,  32,  97,  106,  117,  115,  116,  101,  114,  32,  0,  149,  7,  0,  0,  0,  50,  32,  118,  105,  115,  32,  97,  32,  108,  97,  32,  102,  111,  105,  115,  46,  0,  149,  0,  0,  0,  0,  76,  101,  115,  32,  116,  101,  116,  101,  115,  32,  115,  111,  110,  116,  32,  97,  32,  108,  97,  32,  0,  149,  1,  0,  0,  0,  98,  111,  110,  110,  101,  32,  104,  97,  117,  116,  101,  117,  114,  32,  113,  117,  97,  110,  100,  32,  0,  149,  1,  0,  0,  0,  118,  111,  117,  115,  32,  112,  111,  117,  118,  101,  122,  32,  103,  108,  105,  115,  115,  101,  114,  32,  0,  149,  7,  0,  0,  0,  117,  110,  101,  32,  102,  101,  117,  105,  108,  108,  101,  0,  149,  0,  0,  0,  0,  101,  110,  116,  114,  101,  32,  108,  97,  32,  116,  101,  116,  101,  32,  101,  116,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  108,  97,  32,  112,  108,  97,  113,  117,  101,  32,  99,  104,  97,  117,  102,  102,  97,  110,  116,  101,  0,  149,  1,  0,  0,  0,  80,  114,  101,  110,  101,  122,  32,  117,  110,  101,  32,  102,  101,  117,  105,  108,  108,  101,  32,  32,  0,  149,  7,  0,  0,  0,  112,  111,  117,  114,  32,  118,  111,  117,  115,  32,  97,  105,  100,  101,  114,  46,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  87,  41,  4,  0,  24,  131,  4,  220,  5,  0,  0,  20,  0,  140,  229,  55,  0,  0,  148,  27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  229,  55,  0,  0,  149,  27,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  17,  6,  0,  0,  201,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  218,  223,  55,  0,  24,  142,  18,  6,  0,  0,  202,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  65,  106,  117,  115,  116,  101,  122,  32,  108,  101,  115,  32,  50,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  100,  101,  32,  100,  101,  118,  97,  110,  116,  32,  106,  117,  115,  113,  117,  39,  97,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  111,  117,  118,  111,  105,  114,  32,  103,  108,  105,  115,  115,  101,  114,  32,  108,  97,  32,  32,  0,  149,  7,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  97,  117,  32,  109,  105,  108,  105,  101,  117,  46,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  17,  6,  0,  0,  202,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  17,  6,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  81,  129,  40,  0,  24,  142,  18,  6,  0,  0,  122,  26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  65,  106,  117,  115,  116,  101,  122,  32,  108,  101,  115,  32,  50,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  97,  114,  114,  105,  101,  114,  101,  32,  106,  117,  115,  113,  117,  39,  97,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  111,  117,  118,  111,  105,  114,  32,  103,  108,  105,  115,  115,  101,  114,  32,  108,  97,  32,  32,  0,  149,  7,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  97,  117,  32,  109,  105,  108,  105,  101,  117,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  17,  6,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  42,  39,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  207,  249,  31,  0,  24,  142,  42,  39,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  65,  106,  117,  115,  116,  101,  122,  32,  108,  101,  115,  32,  50,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  100,  101,  32,  100,  114,  111,  105,  116,  101,  32,  106,  117,  115,  113,  117,  39,  97,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  111,  117,  118,  111,  105,  114,  32,  103,  108,  105,  115,  115,  101,  114,  32,  108,  97,  32,  32,  0,  149,  7,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  97,  117,  32,  109,  105,  108,  105,  101,  117,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  41,  39,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  249,  228,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  23,  240,  49,  0,  24,  142,  249,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  65,  106,  117,  115,  116,  101,  122,  32,  108,  101,  115,  32,  50,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  100,  101,  32,  103,  97,  117,  99,  104,  101,  32,  106,  117,  115,  113,  117,  39,  97,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  111,  117,  118,  111,  105,  114,  32,  103,  108,  105,  115,  115,  101,  114,  32,  108,  97,  32,  32,  0,  149,  7,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  97,  117,  32,  109,  105,  108,  105,  101,  117,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  249,  228,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  17,  6,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  11,  248,  24,  0,  24,  142,  17,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  86,  101,  114,  105,  102,  105,  101,  122,  32,  113,  117,  101,  32,  108,  97,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  103,  108,  105,  115,  115,  101,  32,  32,  32,  32,  32,  32,  0,  149,  7,  0,  0,  0,  98,  105,  101,  110,  32,  115,  111,  117,  115,  32,  108,  101,  115,  32,  116,  101,  116,  101,  115,  46,  0,  137,  31 };
#   define LEVEL_PLATE_SINGLE static uint8_t LevelPlateSingle[] PROGMEM = { 149,  0,  0,  0,  0,  84,  111,  117,  114,  110,  101,  122,  32,  108,  101,  115,  32,  52,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  115,  111,  117,  115,  32,  108,  97,  32,  112,  108,  97,  113,  117,  101,  32,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  99,  104,  97,  117,  102,  102,  97,  110,  116,  101,  32,  101,  116,  32,  115,  101,  114,  114,  101,  122,  0,  149,  7,  0,  0,  0,  100,  101,  32,  52,  32,  111,  117,  32,  53,  32,  116,  111,  117,  114,  115,  46,  0,  149,  0,  0,  0,  0,  79,  110,  32,  118,  97,  32,  97,  106,  117,  115,  116,  101,  114,  32,  108,  101,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  116,  101,  116,  101,  115,  44,  32,  106,  101,  32,  118,  97,  105,  115,  32,  100,  111,  110,  99,  32,  0,  149,  1,  0,  0,  0,  108,  101,  115,  32,  100,  101,  112,  108,  97,  99,  101,  114,  32,  100,  97,  110,  115,  32,  32,  32,  0,  149,  7,  0,  0,  0,  100,  105,  102,  102,  101,  114,  101,  110,  116,  101,  115,  32,  112,  111,  115,  105,  116,  46,  0,  149,  0,  0,  0,  0,  80,  111,  117,  114,  32,  99,  104,  97,  113,  117,  101,  32,  112,  111,  115,  105,  116,  44,  32,  32,  0,  149,  1,  0,  0,  0,  110,  111,  117,  115,  32,  97,  108,  108,  111,  110,  115,  32,  97,  106,  117,  115,  116,  101,  114,  32,  0,  149,  7,  0,  0,  0,  50,  32,  118,  105,  115,  32,  97,  32,  108,  97,  32,  102,  111,  105,  115,  46,  0,  149,  0,  0,  0,  0,  76,  101,  115,  32,  116,  101,  116,  101,  115,  32,  115,  111,  110,  116,  32,  97,  32,  108,  97,  32,  0,  149,  1,  0,  0,  0,  98,  111,  110,  110,  101,  32,  104,  97,  117,  116,  101,  117,  114,  32,  113,  117,  97,  110,  100,  32,  0,  149,  1,  0,  0,  0,  118,  111,  117,  115,  32,  112,  111,  117,  118,  101,  122,  32,  103,  108,  105,  115,  115,  101,  114,  32,  0,  149,  7,  0,  0,  0,  117,  110,  101,  32,  102,  101,  117,  105,  108,  108,  101,  0,  149,  0,  0,  0,  0,  101,  110,  116,  114,  101,  32,  108,  97,  32,  116,  101,  116,  101,  32,  101,  116,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  108,  97,  32,  112,  108,  97,  113,  117,  101,  32,  99,  104,  97,  117,  102,  102,  97,  110,  116,  101,  0,  149,  1,  0,  0,  0,  80,  114,  101,  110,  101,  122,  32,  117,  110,  101,  32,  102,  101,  117,  105,  108,  108,  101,  32,  32,  0,  149,  7,  0,  0,  0,  112,  111,  117,  114,  32,  118,  111,  117,  115,  32,  97,  105,  100,  101,  114,  46,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  87,  41,  4,  0,  24,  131,  4,  220,  5,  0,  0,  20,  0,  140,  229,  55,  0,  0,  148,  27,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  229,  55,  0,  0,  148,  27,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  0,  0,  0,  0,  202,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  42,  13,  59,  0,  24,  142,  0,  0,  0,  0,  202,  228,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  65,  106,  117,  115,  116,  101,  122,  32,  108,  101,  115,  32,  50,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  100,  101,  32,  100,  101,  118,  97,  110,  116,  32,  106,  117,  115,  113,  117,  39,  97,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  111,  117,  118,  111,  105,  114,  32,  103,  108,  105,  115,  115,  101,  114,  32,  108,  97,  32,  32,  0,  149,  7,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  97,  117,  32,  109,  105,  108,  105,  101,  117,  46,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  0,  0,  0,  0,  201,  228,  255,  255,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  0,  0,  0,  0,  122,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  81,  129,  40,  0,  24,  142,  0,  0,  0,  0,  122,  26,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  65,  106,  117,  115,  116,  101,  122,  32,  108,  101,  115,  32,  50,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  97,  114,  114,  105,  101,  114,  101,  32,  106,  117,  115,  113,  117,  39,  97,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  111,  117,  118,  111,  105,  114,  32,  103,  108,  105,  115,  115,  101,  114,  32,  108,  97,  32,  32,  0,  149,  7,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  97,  117,  32,  109,  105,  108,  105,  101,  117,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  0,  0,  0,  0,  123,  26,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  198,  36,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  168,  47,  34,  0,  24,  142,  198,  36,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  65,  106,  117,  115,  116,  101,  122,  32,  108,  101,  115,  32,  50,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  100,  101,  32,  100,  114,  111,  105,  116,  101,  32,  106,  117,  115,  113,  117,  39,  97,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  111,  117,  118,  111,  105,  114,  32,  103,  108,  105,  115,  115,  101,  114,  32,  108,  97,  32,  32,  0,  149,  7,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  97,  117,  32,  109,  105,  108,  105,  101,  117,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  198,  36,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  58,  219,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  139,  124,  55,  0,  24,  142,  58,  219,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  65,  106,  117,  115,  116,  101,  122,  32,  108,  101,  115,  32,  50,  32,  118,  105,  115,  32,  32,  32,  0,  149,  1,  0,  0,  0,  100,  101,  32,  103,  97,  117,  99,  104,  101,  32,  106,  117,  115,  113,  117,  39,  97,  32,  32,  32,  0,  149,  1,  0,  0,  0,  112,  111,  117,  118,  111,  105,  114,  32,  103,  108,  105,  115,  115,  101,  114,  32,  108,  97,  32,  32,  0,  149,  7,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  97,  117,  32,  109,  105,  108,  105,  101,  117,  0,  149,  2,  0,  0,  0,  65,  116,  116,  101,  110,  100,  101,  122,  46,  46,  46,  0,  142,  58,  219,  255,  255,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  142,  0,  0,  0,  0,  0,  0,  0,  0,  208,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  69,  190,  27,  0,  24,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  154,  233,  3,  0,  24,  149,  0,  0,  0,  0,  86,  101,  114,  105,  102,  105,  101,  122,  32,  113,  117,  101,  32,  108,  97,  32,  32,  32,  32,  32,  0,  149,  1,  0,  0,  0,  102,  101,  117,  105,  108,  108,  101,  32,  103,  108,  105,  115,  115,  101,  32,  32,  32,  32,  32,  32,  0,  149,  7,  0,  0,  0,  98,  105,  101,  110,  32,  115,  111,  117,  115,  32,  108,  101,  115,  32,  116,  101,  116,  101,  115,  46,  0,  137,  31 };
#   define LEVEL_PLATE_LEN 1540


static PROGMEM prog_uchar SPLASH3_MSG[] = "                    ";
static PROGMEM prog_uchar SPLASH1_MSG[] = "   La Replicator    ";
static PROGMEM prog_uchar SPLASH2_MSG[] = "    ----------      ";
static PROGMEM prog_uchar SPLASH4_MSG[] = "Firmware Version 5. ";

static PROGMEM prog_uchar GO_MSG[] =         "Prechauffer";
static PROGMEM prog_uchar STOP_MSG[] =       "Refroidir";
static PROGMEM prog_uchar RIGHT_TOOL_MSG[] = "Tete de droite";
static PROGMEM prog_uchar LEFT_TOOL_MSG[] =  "Tete de gauche";
static PROGMEM prog_uchar PLATFORM_MSG[] =   "Platforme";
static PROGMEM prog_uchar TOOL_MSG[] =       "Extrudeur";

static PROGMEM prog_uchar START_MSG[] =      "Bonjour !           " "Je suis Replicator  " "   Appuyez sur le M " "rouge pour commencer";
static PROGMEM prog_uchar BUTTONS1_MSG[] =   "Le 'M' clignotant   " "signifie en attente " "de commande pour    " "continuer.          ";
static PROGMEM prog_uchar BUTTONS2_MSG[] =   "Le 'M' fixe signifie" "que je travaille.   " "De nouvelles infos  " "quand travail fini. ";
static PROGMEM prog_uchar EXPLAIN_MSG[] =    "L'etape suivante est" "la configuration.   " "D'abord restaurer la" "plateforme          ";
static PROGMEM prog_uchar LEVEL_MSG[] =      "d'impression.       " "Elle est trop haute " "actuellement.       " "                    ";
static PROGMEM prog_uchar BETTER_MSG[] =     "La plateforme est   " "en bonne place.     " "Mettons du          " "plastique.          ";
static PROGMEM prog_uchar TRYAGAIN_MSG[] =   "Essayez encore...   " "                                                            ";
static PROGMEM prog_uchar GO_ON_MSG[]   =    "Mettez du           " "plastique maintenant" "Pour aide voir      " "makerbot.com/help   ";
static PROGMEM prog_uchar SD_MENU_MSG[] =    "Parfait !           " "Allez dans le menu  " "de la carte SD      " "et choisir un objet.";
static PROGMEM prog_uchar FAIL_MSG[] =       "Allons au menu      " "principal. Pour aide" "voir                " "makerbot.com/help   ";
static PROGMEM prog_uchar START_TEST_MSG[] = "Impression de test  " "pour regler         " "l'alignement des    " "buses.              ";
static PROGMEM prog_uchar EXPLAIN1_MSG[] =   "Choisir le meilleur " "resultat pour chaque" "axe.Les lignes vont " "de 1 a 13 et        ";
static PROGMEM prog_uchar EXPLAIN2_MSG[] =   "la ligne 1 est plus " "longue.             " "Axe Y est a gauche  " "et axe X a droite.  ";
static PROGMEM prog_uchar END_MSG [] =       "Choix enregistres.  " "Imprimez de beaux   " "objets !            " "                    ";

static PROGMEM prog_uchar SELECT_MSG[] =     "Selectionnez les    meilleures lignes.  ";
static PROGMEM prog_uchar DONE_MSG[]   =     "Fini!";
static PROGMEM prog_uchar NO_MSG[]   =       "Non";
static PROGMEM prog_uchar YES_MSG[]  =       "Oui";

static PROGMEM prog_uchar XAXIS_MSG[] =      "Axe X";
static PROGMEM prog_uchar YAXIS_MSG[] =      "Axe Y";


static PROGMEM prog_uchar HEATER_ERROR_MSG[]=  "Les extrudeurs ne   " "chauffent pas.      " "Verifiez les        " "connexions !        ";
static PROGMEM prog_uchar EXPLAIN_ONE_MSG[] =  "Appuyer sur la bague" "grise en haut des   " "extrudeurs et       " "tirez sur le guide. ";
static PROGMEM prog_uchar EXPLAIN_TWO_MSG[] =  "Envoyez du filament " "dans le tube depuis " "l'arriere jusqu'a   " "ce qu'il...         ";
static PROGMEM prog_uchar EXPLAIN_THRE_MSG[]=  "ressorte devant.    " "Extrudeur en chauffe" "Chargement du       " "filament en cours...";
static PROGMEM prog_uchar EXPLAIN_FOUR_MSG[]=  "Ceci peut durer     " "quelques minutes.   " "ATTENTION a ne pas  " "vous bruler !       ";
static PROGMEM prog_uchar EXPLAIN_ONE_S_MSG[]= "Appuyer sur la bague" "grise en haut de    " "l'extrudeur et      " "retirez le guide.   ";
static PROGMEM prog_uchar EXPLAIN_TWO_S_MSG[]= "Envoyez du filament " "dans le tube depuis " "l'arriere jusqu'a   " "ce qu'il...         ";

static PROGMEM prog_uchar HEATING_PROG_MSG[] = "Progression du      " "chauffage :         " "                    " "                    ";
static PROGMEM prog_uchar HEATING_BAR_MSG[] = "Chauffage en cours  " "de l'extrudeur.     " "Merci de patienter. " "                    ";
static PROGMEM prog_uchar READY_RIGHT_MSG[] = "Replicator prete.   " "Chargez du filament " "dans l'extrudeur de " "droite.             ";
static PROGMEM prog_uchar READY_SINGLE_MSG[]= "Replicator prete.   " "Detachez le guide   " "et poussez le       " "filament...        ";
static PROGMEM prog_uchar READY_REV_MSG[]   = "Replicator prete.   " "Detachez le guide et" "retirez doucement le" "filament...        ";
static PROGMEM prog_uchar READY_LEFT_MSG[]  = "Chargez maintenant  " "le filament dans    " "l'extrudeur gauche. " "                    ";
static PROGMEM prog_uchar TUG_MSG[]         = "Dans la bague grise " "jusqu'a ce que le   " "moteur aggripe      " "le fil.             ";
static PROGMEM prog_uchar STOP_MSG_MSG[]    = "Quand le filament   " "sort de la buse,    " "appuyez sur M pour  " "stopper l'extrudeur.";
static PROGMEM prog_uchar STOP_EXIT_MSG[]   = "Quand le filament   " "sort de la buse,    " "appuyez sur M pour  " "quitter.            ";
static PROGMEM prog_uchar STOP_REVERSE_MSG[]= "Quand le filament   " "est totalement sorti" "appuyez sur M pour  " "quitter.            ";
static PROGMEM prog_uchar PUSH_HARDER_MSG[] = "Vous devriez        " "ressayer en         " "appuyant plus fort  " "                    ";
static PROGMEM prog_uchar KEEP_GOING_MSG[]  = "Continuons !        " "Besoin d'aide ?     " "Allez sur           " "makerbot.com/help   ";
static PROGMEM prog_uchar FINISH_MSG[]      = "Arret extrudeur en  " "cours. Appuyez sur M" "pour continuer.     " "                    ";
static PROGMEM prog_uchar GO_ON_LEFT_MSG[]  = "Continuons !        " "Chargez du filament " "dans l'extrudeur    " "gauche...           ";


static PROGMEM prog_uchar READY1_MSG[] =     "Tout va bien ?     ";
static PROGMEM prog_uchar READY2_MSG[] =     "Lancer impression ?";
static PROGMEM prog_uchar NOZZLE_MSG_MSG[] = "La hauteur de ";
static PROGMEM prog_uchar HEIGHT_CHK_MSG[] = "la buse est ok ? ";
static PROGMEM prog_uchar HEIGHT_GOOD_MSG[]= "Bonne hauteur !   ";   // XXX old name: yes[]
static PROGMEM prog_uchar TRY_AGAIN_MSG[]  = "Recommencons.   ";       // XXX old name: no[]

static PROGMEM prog_uchar QONE_MSG[] =       "Du plastique sort-  ";
static PROGMEM prog_uchar QTWO_MSG[] =       "il de l'extrudeur ? ";
static PROGMEM prog_uchar LOAD_RIGHT_MSG[]  ="Mettre a droite"; 
static PROGMEM prog_uchar LOAD_LEFT_MSG[]   ="Mettre a gauche"; 
static PROGMEM prog_uchar LOAD_SINGLE_MSG[] ="Mettre le plastique"; 
static PROGMEM prog_uchar UNLOAD_SINGLE_MSG[]="Retirer le plastique"; 
static PROGMEM prog_uchar UNLOAD_RIGHT_MSG[]="Retirer a gauche"; 
static PROGMEM prog_uchar UNLOAD_LEFT_MSG[] ="Retirer a droite"; 

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

static PROGMEM prog_uchar DISTANCESHORT_MSG[] = "COURT";
static PROGMEM prog_uchar DISTANCELONG_MSG[] =  "LONG";
static PROGMEM prog_uchar GAMEOVER_MSG[] =      "PERDU !";

static PROGMEM prog_uchar HEATING_MSG[] =        "Chauffe:";
static PROGMEM prog_uchar HEATING_SPACES_MSG[] = "Chauffe:            ";

static PROGMEM prog_uchar BUILD_PERCENT_MSG[] =    " --%";
static PROGMEM prog_uchar EXTRUDER1_TEMP_MSG[] =   "Tete Droite ---/---C";
static PROGMEM prog_uchar EXTRUDER2_TEMP_MSG[] =   "Tete Gauche ---/---C";
static PROGMEM prog_uchar PLATFORM_TEMP_MSG[]  =   "Plateforme  ---/---C";
static PROGMEM prog_uchar EXTRUDER_TEMP_MSG[]  =   "Extrudeur:  ---/---C";

static PROGMEM prog_uchar PREHEAT_SET_MSG[] = "Param. prechauffage ";
static PROGMEM prog_uchar RIGHT_SPACES_MSG[] =  "Tete de droite     ";
static PROGMEM prog_uchar LEFT_SPACES_MSG[]   = "Tete de gauche     ";
static PROGMEM prog_uchar PLATFORM_SPACES_MSG[]  = "Platforme       ";
static PROGMEM prog_uchar RESET1_MSG[] = "  Retour parametres  ";
static PROGMEM prog_uchar RESET2_MSG[] = "     d'usine ?      ";
static PROGMEM prog_uchar CANCEL_MSG[] = "Annuler impression?";
static PROGMEM prog_uchar CANCEL_PROCESS_MSG[] = "Quitter processus? ";

static PROGMEM prog_uchar PAUSE_MSG[] = "Pause";
static PROGMEM prog_uchar UNPAUSE_MSG[] = "Reprendre";

static PROGMEM prog_uchar NOCARD_MSG[] = "CarteSD introuvable";

static PROGMEM prog_uchar BUILD_MSG[] =            "Imprimer depuis SD";
static PROGMEM prog_uchar PREHEAT_MSG[] =          "Prechauffage";
static PROGMEM prog_uchar UTILITIES_MSG[] =        "Utilitaires";
static PROGMEM prog_uchar MONITOR_MSG[] =          "Visu Temp   ";
static PROGMEM prog_uchar JOG_MSG[]   =            "Mode Manuel";
static PROGMEM prog_uchar CALIBRATION_MSG[] =      "Calibration des axes";
static PROGMEM prog_uchar HOME_AXES_MSG[] =        "Axes a l'origine";
static PROGMEM prog_uchar FILAMENT_OPTIONS_MSG[] = "Options du filament";
static PROGMEM prog_uchar STARTUP_MSG[] =          "Script de Demarrage";
static PROGMEM prog_uchar DSTEPS_MSG[] =           "Debloquer Moteurs";
static PROGMEM prog_uchar ESTEPS_MSG[] =           "Bloquer Moteurs  ";
static PROGMEM prog_uchar PLATE_LEVEL_MSG[] =      "Relever Plateforme";
static PROGMEM prog_uchar LED_BLINK_MSG[] =        "Clignotement LED ";
static PROGMEM prog_uchar LED_STOP_MSG[] =         "Stop Clignotement";
static PROGMEM prog_uchar PREHEAT_SETTINGS_MSG[] = "Param. prechauffage ";
static PROGMEM prog_uchar VERSION_MSG[] = 		   "Numero de Version";
static PROGMEM prog_uchar SETTINGS_MSG[] =         "Parametres Generaux";
static PROGMEM prog_uchar RESET_MSG[] =            "RAZ usine";
static PROGMEM prog_uchar NOZZLES_MSG[] =          "Calibration Buses";
static PROGMEM prog_uchar TOOL_COUNT_MSG[]  =      "Nb Tetes:  ";
static PROGMEM prog_uchar SOUND_MSG[] =            "Son";
static PROGMEM prog_uchar LED_MSG[] =              "Eclairage     ";
static PROGMEM prog_uchar LED_HEAT_MSG[] =         "Coul Chauf";
static PROGMEM prog_uchar HELP_SCREENS_MSG[] =     "Aff. aide ";
static PROGMEM prog_uchar EXIT_MSG[] =             "Sortir du Menu";
static PROGMEM prog_uchar ACCELERATE_MSG[] = 	   "Accelerer";

static PROGMEM prog_uchar RED_COLOR_MSG[]    = "ROUGE ";
static PROGMEM prog_uchar ORANGE_COLOR_MSG[] = "ORANGE ";
static PROGMEM prog_uchar PINK_COLOR_MSG[]   = "ROSE  ";
static PROGMEM prog_uchar GREEN_COLOR_MSG[]  = "VERT  ";
static PROGMEM prog_uchar BLUE_COLOR_MSG[]   = "BLEU  ";
static PROGMEM prog_uchar PURPLE_COLOR_MSG[] = "VIOLET";
static PROGMEM prog_uchar WHITE_COLOR_MSG[]  = "BLANC ";
static PROGMEM prog_uchar CUSTOM_COLOR_MSG[] = "AUTRE ";

static PROGMEM prog_uchar TOOL_SINGLE_MSG[] = "SIMPLE";
static PROGMEM prog_uchar TOOL_DUAL_MSG[] =   "DOUBLE";

static PROGMEM prog_uchar RIGHT_MSG[] =   "Droite";
static PROGMEM prog_uchar LEFT_MSG[] =    "Gauche";
static PROGMEM prog_uchar ERROR_MSG[] =   "erreur";

static PROGMEM prog_uchar NA_MSG[] =      "  NA    ";
static PROGMEM prog_uchar WAITING_MSG[] = "attente ";

#define HEATER_INACTIVITY_MSG           "Exctincion des tetes" "due a l'inactivite  "
#define HEATER_FAIL_SOFTWARE_CUTOFF_MSG "Surchauffe des tetes" "Temperature limite  " "Atteinte! Veuillez  " "Redemarrer          "
#define HEATER_FAIL_HARDWARE_CUTOFF_MSG "Surchauffe des tetes" "Extinction securite " "Declenchee! Veuillez" "Redemarrer          "
#define HEATER_FAIL_NOT_HEATING_MSG     "Surchauffe des tetes" "Elles ne chauffent  " "pas correctement !  " "Verif. connectiques "
#define HEATER_FAIL_DROPPING_TEMP_MSG   "Echec du chauffage !" "La temperature des  " "tetes chute !       " "Verif. connectiques "
#define HEATER_FAIL_NOT_PLUGGED_IN_MSG  "Erreur de chauffe ! " "Echec du releve de  " "temperature!        " "Verif. connectiques "



