/*
 * Thermistor tables harvested from Marlin (based on Sprinter and grbl)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef THERMISTOR_TABLES_H_
#define THERMISTOR_TABLES_H_

#if BOARD_TYPE == BOARD_TYPE_AZTEEG_X3

// Marlin indices
//
//  1 - Generic 100K NTC thermistor (beta 4338K)
//  2 - 200K ATC Semitex 204GT-2
//  3 - "Mendel Parts"
//  4 - Generic 10k NTC thermistor
//  5 - 100K ATC Semitec 104GT-2 (used in ParCan; beta 4267)
//  6 - 100K Epcos
//  7 - 100K Honeywell 135-104LAG-J01
// 71 - 100K Honeywell 135-104LAF-J01
//  8 - 100K 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup)
//  9 - 100K GE Sensing AL03006-58.2K-97-G1 (4.7k pullup)
// 10 - 100K RS thermistor 198-961 (4.7k pullup)
// 11 - QU-BD silicone bed QWG-104F-3950 thermistor

#define TEMP_TABLE_SIZE(t) (sizeof(t) / sizeof(Entry))

// 100k bed thermistor

#if MY_INDEX == 1

const static PROGMEM prog_uchar MY_NAME[] = "1. Generic 100K";

static const Entry MY_TABLE[] PROGMEM = {
    {   23 * TEMP_OVERSAMPLE, 300 },
    {   25 * TEMP_OVERSAMPLE, 295 },
    {   27 * TEMP_OVERSAMPLE, 290 },
    {   28 * TEMP_OVERSAMPLE, 285 },
    {   31 * TEMP_OVERSAMPLE, 280 },
    {   33 * TEMP_OVERSAMPLE, 275 },
    {   35 * TEMP_OVERSAMPLE, 270 },
    {   38 * TEMP_OVERSAMPLE, 265 },
    {   41 * TEMP_OVERSAMPLE, 260 },
    {   44 * TEMP_OVERSAMPLE, 255 },
    {   48 * TEMP_OVERSAMPLE, 250 },
    {   52 * TEMP_OVERSAMPLE, 245 },
    {   56 * TEMP_OVERSAMPLE, 240 },
    {   61 * TEMP_OVERSAMPLE, 235 },
    {   66 * TEMP_OVERSAMPLE, 230 },
    {   71 * TEMP_OVERSAMPLE, 225 },
    {   78 * TEMP_OVERSAMPLE, 220 },
    {   84 * TEMP_OVERSAMPLE, 215 },
    {   92 * TEMP_OVERSAMPLE, 210 },
    {  100 * TEMP_OVERSAMPLE, 205 },
    {  109 * TEMP_OVERSAMPLE, 200 },
    {  120 * TEMP_OVERSAMPLE, 195 },
    {  131 * TEMP_OVERSAMPLE, 190 },
    {  143 * TEMP_OVERSAMPLE, 185 },
    {  156 * TEMP_OVERSAMPLE, 180 },
    {  171 * TEMP_OVERSAMPLE, 175 },
    {  187 * TEMP_OVERSAMPLE, 170 },
    {  205 * TEMP_OVERSAMPLE, 165 },
    {  224 * TEMP_OVERSAMPLE, 160 },
    {  245 * TEMP_OVERSAMPLE, 155 },
    {  268 * TEMP_OVERSAMPLE, 150 },
    {  293 * TEMP_OVERSAMPLE, 145 },
    {  320 * TEMP_OVERSAMPLE, 140 },
    {  348 * TEMP_OVERSAMPLE, 135 },
    {  379 * TEMP_OVERSAMPLE, 130 },
    {  411 * TEMP_OVERSAMPLE, 125 },
    {  445 * TEMP_OVERSAMPLE, 120 },
    {  480 * TEMP_OVERSAMPLE, 115 },
    {  516 * TEMP_OVERSAMPLE, 110 },
    {  553 * TEMP_OVERSAMPLE, 105 },
    {  591 * TEMP_OVERSAMPLE, 100 },
    {  628 * TEMP_OVERSAMPLE,  95 },
    {  665 * TEMP_OVERSAMPLE,  90 },
    {  702 * TEMP_OVERSAMPLE,  85 },
    {  737 * TEMP_OVERSAMPLE,  80 },
    {  770 * TEMP_OVERSAMPLE,  75 },
    {  801 * TEMP_OVERSAMPLE,  70 },
    {  830 * TEMP_OVERSAMPLE,  65 },
    {  857 * TEMP_OVERSAMPLE,  60 },
    {  881 * TEMP_OVERSAMPLE,  55 },
    {  903 * TEMP_OVERSAMPLE,  50 },
    {  922 * TEMP_OVERSAMPLE,  45 },
    {  939 * TEMP_OVERSAMPLE,  40 },
    {  954 * TEMP_OVERSAMPLE,  35 },
    {  966 * TEMP_OVERSAMPLE,  30 },
    {  977 * TEMP_OVERSAMPLE,  25 },
    {  985 * TEMP_OVERSAMPLE,  20 },
    {  993 * TEMP_OVERSAMPLE,  15 },
    {  999 * TEMP_OVERSAMPLE,  10 },
    { 1004 * TEMP_OVERSAMPLE,   5 },
    { 1008 * TEMP_OVERSAMPLE,   0 } //safety
};

#endif

#if MY_INDEX == 2

// 200k thermistor
// 200k ATC Semitec 204GT-2
// Beta 4338K
// Verified by linagee.
// Source: http://shop.arcol.hu/static/datasheets/thermistors.pdf
// Calculated using 4.7kohm pullup, voltage divider math,
// and manufacturer provided temp/resistance

const static PROGMEM prog_uchar MY_NAME[] = "2. Semitec 204GT-2";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 848},
    {   30 * TEMP_OVERSAMPLE, 300}, //top rating 300C
    {   34 * TEMP_OVERSAMPLE, 290},
    {   39 * TEMP_OVERSAMPLE, 280},
    {   46 * TEMP_OVERSAMPLE, 270},
    {   53 * TEMP_OVERSAMPLE, 260},
    {   63 * TEMP_OVERSAMPLE, 250},
    {   74 * TEMP_OVERSAMPLE, 240},
    {   87 * TEMP_OVERSAMPLE, 230},
    {  104 * TEMP_OVERSAMPLE, 220},
    {  124 * TEMP_OVERSAMPLE, 210},
    {  148 * TEMP_OVERSAMPLE, 200},
    {  176 * TEMP_OVERSAMPLE, 190},
    {  211 * TEMP_OVERSAMPLE, 180},
    {  252 * TEMP_OVERSAMPLE, 170},
    {  301 * TEMP_OVERSAMPLE, 160},
    {  357 * TEMP_OVERSAMPLE, 150},
    {  420 * TEMP_OVERSAMPLE, 140},
    {  489 * TEMP_OVERSAMPLE, 130},
    {  562 * TEMP_OVERSAMPLE, 120},
    {  636 * TEMP_OVERSAMPLE, 110},
    {  708 * TEMP_OVERSAMPLE, 100},
    {  775 * TEMP_OVERSAMPLE,  90},
    {  835 * TEMP_OVERSAMPLE,  80},
    {  884 * TEMP_OVERSAMPLE,  70},
    {  924 * TEMP_OVERSAMPLE,  60},
    {  955 * TEMP_OVERSAMPLE,  50},
    {  977 * TEMP_OVERSAMPLE,  40},
    {  993 * TEMP_OVERSAMPLE,  30},
    { 1004 * TEMP_OVERSAMPLE,  20},
    { 1012 * TEMP_OVERSAMPLE,  10},
    { 1016 * TEMP_OVERSAMPLE,   0}
};

#endif

#if MY_INDEX == 3

//mendel-parts

const static PROGMEM prog_uchar MY_NAME[] = "3. Mendel";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 864},
    {   21 * TEMP_OVERSAMPLE, 300},
    {   25 * TEMP_OVERSAMPLE, 290},
    {   29 * TEMP_OVERSAMPLE, 280},
    {   33 * TEMP_OVERSAMPLE, 270},
    {   39 * TEMP_OVERSAMPLE, 260},
    {   46 * TEMP_OVERSAMPLE, 250},
    {   54 * TEMP_OVERSAMPLE, 240},
    {   64 * TEMP_OVERSAMPLE, 230},
    {   75 * TEMP_OVERSAMPLE, 220},
    {   90 * TEMP_OVERSAMPLE, 210},
    {  107 * TEMP_OVERSAMPLE, 200},
    {  128 * TEMP_OVERSAMPLE, 190},
    {  154 * TEMP_OVERSAMPLE, 180},
    {  184 * TEMP_OVERSAMPLE, 170},
    {  221 * TEMP_OVERSAMPLE, 160},
    {  265 * TEMP_OVERSAMPLE, 150},
    {  316 * TEMP_OVERSAMPLE, 140},
    {  375 * TEMP_OVERSAMPLE, 130},
    {  441 * TEMP_OVERSAMPLE, 120},
    {  513 * TEMP_OVERSAMPLE, 110},
    {  588 * TEMP_OVERSAMPLE, 100},
    {  734 * TEMP_OVERSAMPLE,  80},
    {  856 * TEMP_OVERSAMPLE,  60},
    {  938 * TEMP_OVERSAMPLE,  40},
    {  986 * TEMP_OVERSAMPLE,  20},
    { 1008 * TEMP_OVERSAMPLE,   0},
    { 1018 * TEMP_OVERSAMPLE, -20}
};

#endif

#if MY_INDEX == 4

// 10k thermistor

const static PROGMEM prog_uchar MY_NAME[] = "4. Generic 10K";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 430},
    {   54 * TEMP_OVERSAMPLE, 137},
    {  107 * TEMP_OVERSAMPLE, 107},
    {  160 * TEMP_OVERSAMPLE,  91},
    {  213 * TEMP_OVERSAMPLE,  80},
    {  266 * TEMP_OVERSAMPLE,  71},
    {  319 * TEMP_OVERSAMPLE,  64},
    {  372 * TEMP_OVERSAMPLE,  57},
    {  425 * TEMP_OVERSAMPLE,  51},
    {  478 * TEMP_OVERSAMPLE,  46},
    {  531 * TEMP_OVERSAMPLE,  41},
    {  584 * TEMP_OVERSAMPLE,  35},
    {  637 * TEMP_OVERSAMPLE,  30},
    {  690 * TEMP_OVERSAMPLE,  25},
    {  743 * TEMP_OVERSAMPLE,  20},
    {  796 * TEMP_OVERSAMPLE,  14},
    {  849 * TEMP_OVERSAMPLE,   7},
    {  902 * TEMP_OVERSAMPLE,   0},
    {  955 * TEMP_OVERSAMPLE, -11},
    { 1008 * TEMP_OVERSAMPLE, -35}
};

#endif

#if MY_INDEX == 5

//100k ParCan thermistor (104GT-2)
// ATC Semitec 104GT-2 (Used in ParCan)
// Beta 4267
// Verified by linagee
// Source: http://shop.arcol.hu/static/datasheets/thermistors.pdf
// Calculated using 4.7kohm pullup, voltage divider math,
// and manufacturer provided temp/resistance

const static PROGMEM prog_uchar MY_NAME[] = "5. Semitec 104GT-2";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 713},
    {   17 * TEMP_OVERSAMPLE, 300},//top rating 300C
    {   20 * TEMP_OVERSAMPLE, 290},
    {   23 * TEMP_OVERSAMPLE, 280},
    {   27 * TEMP_OVERSAMPLE, 270},
    {   31 * TEMP_OVERSAMPLE, 260},
    {   37 * TEMP_OVERSAMPLE, 250},
    {   43 * TEMP_OVERSAMPLE, 240},
    {   51 * TEMP_OVERSAMPLE, 230},
    {   61 * TEMP_OVERSAMPLE, 220},
    {   73 * TEMP_OVERSAMPLE, 210},
    {   87 * TEMP_OVERSAMPLE, 200},
    {  106 * TEMP_OVERSAMPLE, 190},
    {  128 * TEMP_OVERSAMPLE, 180},
    {  155 * TEMP_OVERSAMPLE, 170},
    {  189 * TEMP_OVERSAMPLE, 160},
    {  230 * TEMP_OVERSAMPLE, 150},
    {  278 * TEMP_OVERSAMPLE, 140},
    {  336 * TEMP_OVERSAMPLE, 130},
    {  402 * TEMP_OVERSAMPLE, 120},
    {  476 * TEMP_OVERSAMPLE, 110},
    {  554 * TEMP_OVERSAMPLE, 100},
    {  635 * TEMP_OVERSAMPLE,  90},
    {  713 * TEMP_OVERSAMPLE,  80},
    {  784 * TEMP_OVERSAMPLE,  70},
    {  846 * TEMP_OVERSAMPLE,  60},
    {  897 * TEMP_OVERSAMPLE,  50},
    {  937 * TEMP_OVERSAMPLE,  40},
    {  966 * TEMP_OVERSAMPLE,  30},
    {  986 * TEMP_OVERSAMPLE,  20},
    { 1000 * TEMP_OVERSAMPLE,  10},
    { 1010 * TEMP_OVERSAMPLE,   0}
};

#endif

#if MY_INDEX == 6

// 100k Epcos thermistor

const static PROGMEM prog_uchar MY_NAME[] = "6. Epcos 100K";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 350},
    {   28 * TEMP_OVERSAMPLE, 250},//top rating 250C
    {   31 * TEMP_OVERSAMPLE, 245},
    {   35 * TEMP_OVERSAMPLE, 240},
    {   39 * TEMP_OVERSAMPLE, 235},
    {   42 * TEMP_OVERSAMPLE, 230},
    {   44 * TEMP_OVERSAMPLE, 225},
    {   49 * TEMP_OVERSAMPLE, 220},
    {   53 * TEMP_OVERSAMPLE, 215},
    {   62 * TEMP_OVERSAMPLE, 210},
    {   71 * TEMP_OVERSAMPLE, 205},//fitted graphically
    {   78 * TEMP_OVERSAMPLE, 200},//fitted graphically
    {   94 * TEMP_OVERSAMPLE, 190},
    {  102 * TEMP_OVERSAMPLE, 185},
    {  116 * TEMP_OVERSAMPLE, 170},
    {  143 * TEMP_OVERSAMPLE, 160},
    {  183 * TEMP_OVERSAMPLE, 150},
    {  223 * TEMP_OVERSAMPLE, 140},
    {  270 * TEMP_OVERSAMPLE, 130},
    {  318 * TEMP_OVERSAMPLE, 120},
    {  383 * TEMP_OVERSAMPLE, 110},
    {  413 * TEMP_OVERSAMPLE, 105},
    {  439 * TEMP_OVERSAMPLE, 100},
    {  484 * TEMP_OVERSAMPLE,  95},
    {  513 * TEMP_OVERSAMPLE,  90},
    {  607 * TEMP_OVERSAMPLE,  80},
    {  664 * TEMP_OVERSAMPLE,  70},
    {  781 * TEMP_OVERSAMPLE,  60},
    {  810 * TEMP_OVERSAMPLE,  55},
    {  849 * TEMP_OVERSAMPLE,  50},
    {  914 * TEMP_OVERSAMPLE,  45},
    {  914 * TEMP_OVERSAMPLE,  40},
    {  935 * TEMP_OVERSAMPLE,  35},
    {  954 * TEMP_OVERSAMPLE,  30},
    {  970 * TEMP_OVERSAMPLE,  25},
    {  978 * TEMP_OVERSAMPLE,  22},
    { 1008 * TEMP_OVERSAMPLE,   3},
    { 1023 * TEMP_OVERSAMPLE,   0}  //to allow internal 0 degrees C
};

#endif

#if MY_INDEX == 7

// 100k Honeywell 135-104LAG-J01

const static PROGMEM prog_uchar MY_NAME[] = "7. Hw 135-104LAG-J01";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 941},
    {   19 * TEMP_OVERSAMPLE, 362},
    {   37 * TEMP_OVERSAMPLE, 299},//top rating 300C
    {   55 * TEMP_OVERSAMPLE, 266},
    {   73 * TEMP_OVERSAMPLE, 245},
    {   91 * TEMP_OVERSAMPLE, 229},
    {  109 * TEMP_OVERSAMPLE, 216},
    {  127 * TEMP_OVERSAMPLE, 206},
    {  145 * TEMP_OVERSAMPLE, 197},
    {  163 * TEMP_OVERSAMPLE, 190},
    {  181 * TEMP_OVERSAMPLE, 183},
    {  199 * TEMP_OVERSAMPLE, 177},
    {  217 * TEMP_OVERSAMPLE, 171},
    {  235 * TEMP_OVERSAMPLE, 166},
    {  253 * TEMP_OVERSAMPLE, 162},
    {  271 * TEMP_OVERSAMPLE, 157},
    {  289 * TEMP_OVERSAMPLE, 153},
    {  307 * TEMP_OVERSAMPLE, 149},
    {  325 * TEMP_OVERSAMPLE, 146},
    {  343 * TEMP_OVERSAMPLE, 142},
    {  361 * TEMP_OVERSAMPLE, 139},
    {  379 * TEMP_OVERSAMPLE, 135},
    {  397 * TEMP_OVERSAMPLE, 132},
    {  415 * TEMP_OVERSAMPLE, 129},
    {  433 * TEMP_OVERSAMPLE, 126},
    {  451 * TEMP_OVERSAMPLE, 123},
    {  469 * TEMP_OVERSAMPLE, 121},
    {  487 * TEMP_OVERSAMPLE, 118},
    {  505 * TEMP_OVERSAMPLE, 115},
    {  523 * TEMP_OVERSAMPLE, 112},
    {  541 * TEMP_OVERSAMPLE, 110},
    {  559 * TEMP_OVERSAMPLE, 107},
    {  577 * TEMP_OVERSAMPLE, 105},
    {  595 * TEMP_OVERSAMPLE, 102},
    {  613 * TEMP_OVERSAMPLE,  99},
    {  631 * TEMP_OVERSAMPLE,  97},
    {  649 * TEMP_OVERSAMPLE,  94},
    {  667 * TEMP_OVERSAMPLE,  92},
    {  685 * TEMP_OVERSAMPLE,  89},
    {  703 * TEMP_OVERSAMPLE,  86},
    {  721 * TEMP_OVERSAMPLE,  84},
    {  739 * TEMP_OVERSAMPLE,  81},
    {  757 * TEMP_OVERSAMPLE,  78},
    {  775 * TEMP_OVERSAMPLE,  75},
    {  793 * TEMP_OVERSAMPLE,  72},
    {  811 * TEMP_OVERSAMPLE,  69},
    {  829 * TEMP_OVERSAMPLE,  66},
    {  847 * TEMP_OVERSAMPLE,  62},
    {  865 * TEMP_OVERSAMPLE,  59},
    {  883 * TEMP_OVERSAMPLE,  55},
    {  901 * TEMP_OVERSAMPLE,  51},
    {  919 * TEMP_OVERSAMPLE,  46},
    {  937 * TEMP_OVERSAMPLE,  41},
    {  955 * TEMP_OVERSAMPLE,  35},
    {  973 * TEMP_OVERSAMPLE,  27},
    {  991 * TEMP_OVERSAMPLE,  17},
    { 1009 * TEMP_OVERSAMPLE,   1},
    { 1023 * TEMP_OVERSAMPLE,   0}  //to allow internal 0 degrees C
};

#endif

#if MY_INDEX == 71

// 100k Honeywell 135-104LAF-J01
// R0 = 100000 Ohm
// T0 = 25 C
// Beta = 3974
// R1 = 0 Ohm
// R2 = 4700 Ohm

const static PROGMEM prog_uchar MY_NAME[] = "71.Hw 135-104LAF-J01";

static const Entry MY_TABLE[] PROGMEM = {
    {   35 * TEMP_OVERSAMPLE, 300},
    {   51 * TEMP_OVERSAMPLE, 270},
    {   54 * TEMP_OVERSAMPLE, 265},
    {   58 * TEMP_OVERSAMPLE, 260},
    {   59 * TEMP_OVERSAMPLE, 258},
    {   61 * TEMP_OVERSAMPLE, 256},
    {   63 * TEMP_OVERSAMPLE, 254},
    {   64 * TEMP_OVERSAMPLE, 252},
    {   66 * TEMP_OVERSAMPLE, 250},
    {   67 * TEMP_OVERSAMPLE, 249},
    {   68 * TEMP_OVERSAMPLE, 248},
    {   69 * TEMP_OVERSAMPLE, 247},
    {   70 * TEMP_OVERSAMPLE, 246},
    {   71 * TEMP_OVERSAMPLE, 245},
    {   72 * TEMP_OVERSAMPLE, 244},
    {   73 * TEMP_OVERSAMPLE, 243},
    {   74 * TEMP_OVERSAMPLE, 242},
    {   75 * TEMP_OVERSAMPLE, 241},
    {   76 * TEMP_OVERSAMPLE, 240},
    {   77 * TEMP_OVERSAMPLE, 239},
    {   78 * TEMP_OVERSAMPLE, 238},
    {   79 * TEMP_OVERSAMPLE, 237},
    {   80 * TEMP_OVERSAMPLE, 236},
    {   81 * TEMP_OVERSAMPLE, 235},
    {   82 * TEMP_OVERSAMPLE, 234},
    {   84 * TEMP_OVERSAMPLE, 233},
    {   85 * TEMP_OVERSAMPLE, 232},
    {   86 * TEMP_OVERSAMPLE, 231},
    {   87 * TEMP_OVERSAMPLE, 230},
    {   89 * TEMP_OVERSAMPLE, 229},
    {   90 * TEMP_OVERSAMPLE, 228},
    {   91 * TEMP_OVERSAMPLE, 227},
    {   92 * TEMP_OVERSAMPLE, 226},
    {   94 * TEMP_OVERSAMPLE, 225},
    {   95 * TEMP_OVERSAMPLE, 224},
    {   97 * TEMP_OVERSAMPLE, 223},
    {   98 * TEMP_OVERSAMPLE, 222},
    {   99 * TEMP_OVERSAMPLE, 221},
    {  101 * TEMP_OVERSAMPLE, 220},
    {  102 * TEMP_OVERSAMPLE, 219},
    {  104 * TEMP_OVERSAMPLE, 218},
    {  106 * TEMP_OVERSAMPLE, 217},
    {  107 * TEMP_OVERSAMPLE, 216},
    {  109 * TEMP_OVERSAMPLE, 215},
    {  110 * TEMP_OVERSAMPLE, 214},
    {  112 * TEMP_OVERSAMPLE, 213},
    {  114 * TEMP_OVERSAMPLE, 212},
    {  115 * TEMP_OVERSAMPLE, 211},
    {  117 * TEMP_OVERSAMPLE, 210},
    {  119 * TEMP_OVERSAMPLE, 209},
    {  121 * TEMP_OVERSAMPLE, 208},
    {  123 * TEMP_OVERSAMPLE, 207},
    {  125 * TEMP_OVERSAMPLE, 206},
    {  126 * TEMP_OVERSAMPLE, 205},
    {  128 * TEMP_OVERSAMPLE, 204},
    {  130 * TEMP_OVERSAMPLE, 203},
    {  132 * TEMP_OVERSAMPLE, 202},
    {  134 * TEMP_OVERSAMPLE, 201},
    {  136 * TEMP_OVERSAMPLE, 200},
    {  139 * TEMP_OVERSAMPLE, 199},
    {  141 * TEMP_OVERSAMPLE, 198},
    {  143 * TEMP_OVERSAMPLE, 197},
    {  145 * TEMP_OVERSAMPLE, 196},
    {  147 * TEMP_OVERSAMPLE, 195},
    {  150 * TEMP_OVERSAMPLE, 194},
    {  152 * TEMP_OVERSAMPLE, 193},
    {  154 * TEMP_OVERSAMPLE, 192},
    {  157 * TEMP_OVERSAMPLE, 191},
    {  159 * TEMP_OVERSAMPLE, 190},
    {  162 * TEMP_OVERSAMPLE, 189},
    {  164 * TEMP_OVERSAMPLE, 188},
    {  167 * TEMP_OVERSAMPLE, 187},
    {  170 * TEMP_OVERSAMPLE, 186},
    {  172 * TEMP_OVERSAMPLE, 185},
    {  175 * TEMP_OVERSAMPLE, 184},
    {  178 * TEMP_OVERSAMPLE, 183},
    {  181 * TEMP_OVERSAMPLE, 182},
    {  184 * TEMP_OVERSAMPLE, 181},
    {  187 * TEMP_OVERSAMPLE, 180},
    {  190 * TEMP_OVERSAMPLE, 179},
    {  193 * TEMP_OVERSAMPLE, 178},
    {  196 * TEMP_OVERSAMPLE, 177},
    {  199 * TEMP_OVERSAMPLE, 176},
    {  202 * TEMP_OVERSAMPLE, 175},
    {  205 * TEMP_OVERSAMPLE, 174},
    {  208 * TEMP_OVERSAMPLE, 173},
    {  212 * TEMP_OVERSAMPLE, 172},
    {  215 * TEMP_OVERSAMPLE, 171},
    {  219 * TEMP_OVERSAMPLE, 170},
    {  237 * TEMP_OVERSAMPLE, 165},
    {  256 * TEMP_OVERSAMPLE, 160},
    {  300 * TEMP_OVERSAMPLE, 150},
    {  351 * TEMP_OVERSAMPLE, 140},
    {  470 * TEMP_OVERSAMPLE, 120},
    {  504 * TEMP_OVERSAMPLE, 115},
    {  538 * TEMP_OVERSAMPLE, 110},
    {  552 * TEMP_OVERSAMPLE, 108},
    {  566 * TEMP_OVERSAMPLE, 106},
    {  580 * TEMP_OVERSAMPLE, 104},
    {  594 * TEMP_OVERSAMPLE, 102},
    {  608 * TEMP_OVERSAMPLE, 100},
    {  622 * TEMP_OVERSAMPLE,  98},
    {  636 * TEMP_OVERSAMPLE,  96},
    {  650 * TEMP_OVERSAMPLE,  94},
    {  664 * TEMP_OVERSAMPLE,  92},
    {  678 * TEMP_OVERSAMPLE,  90},
    {  712 * TEMP_OVERSAMPLE,  85},
    {  745 * TEMP_OVERSAMPLE,  80},
    {  758 * TEMP_OVERSAMPLE,  78},
    {  770 * TEMP_OVERSAMPLE,  76},
    {  783 * TEMP_OVERSAMPLE,  74},
    {  795 * TEMP_OVERSAMPLE,  72},
    {  806 * TEMP_OVERSAMPLE,  70},
    {  818 * TEMP_OVERSAMPLE,  68},
    {  829 * TEMP_OVERSAMPLE,  66},
    {  840 * TEMP_OVERSAMPLE,  64},
    {  850 * TEMP_OVERSAMPLE,  62},
    {  860 * TEMP_OVERSAMPLE,  60},
    {  870 * TEMP_OVERSAMPLE,  58},
    {  879 * TEMP_OVERSAMPLE,  56},
    {  888 * TEMP_OVERSAMPLE,  54},
    {  897 * TEMP_OVERSAMPLE,  52},
    {  905 * TEMP_OVERSAMPLE,  50},
    {  924 * TEMP_OVERSAMPLE,  45},
    {  940 * TEMP_OVERSAMPLE,  40},
    {  955 * TEMP_OVERSAMPLE,  35},
    {  967 * TEMP_OVERSAMPLE,  30},
    {  970 * TEMP_OVERSAMPLE,  29},
    {  972 * TEMP_OVERSAMPLE,  28},
    {  974 * TEMP_OVERSAMPLE,  27},
    {  976 * TEMP_OVERSAMPLE,  26},
    {  978 * TEMP_OVERSAMPLE,  25},
    {  980 * TEMP_OVERSAMPLE,  24},
    {  982 * TEMP_OVERSAMPLE,  23},
    {  984 * TEMP_OVERSAMPLE,  22},
    {  985 * TEMP_OVERSAMPLE,  21},
    {  987 * TEMP_OVERSAMPLE,  20},
    {  995 * TEMP_OVERSAMPLE,  15},
    { 1001 * TEMP_OVERSAMPLE,  10},
    { 1006 * TEMP_OVERSAMPLE,   5},
    { 1010 * TEMP_OVERSAMPLE,   0},
};

#endif

#if MY_INDEX == 8

// 100k 0603 SMD Vishay NTCS0603E3104FXT (4.7k pullup)

const static PROGMEM prog_uchar MY_NAME[] = "8. Vishay E3104FXT";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 704},
    {   54 * TEMP_OVERSAMPLE, 216},
    {  107 * TEMP_OVERSAMPLE, 175},
    {  160 * TEMP_OVERSAMPLE, 152},
    {  213 * TEMP_OVERSAMPLE, 137},
    {  266 * TEMP_OVERSAMPLE, 125},
    {  319 * TEMP_OVERSAMPLE, 115},
    {  372 * TEMP_OVERSAMPLE, 106},
    {  425 * TEMP_OVERSAMPLE,  99},
    {  478 * TEMP_OVERSAMPLE,  91},
    {  531 * TEMP_OVERSAMPLE,  85},
    {  584 * TEMP_OVERSAMPLE,  78},
    {  637 * TEMP_OVERSAMPLE,  71},
    {  690 * TEMP_OVERSAMPLE,  65},
    {  743 * TEMP_OVERSAMPLE,  58},
    {  796 * TEMP_OVERSAMPLE,  50},
    {  849 * TEMP_OVERSAMPLE,  42},
    {  902 * TEMP_OVERSAMPLE,  31},
    {  955 * TEMP_OVERSAMPLE,  17},
    { 1008 * TEMP_OVERSAMPLE,   0}
};

#endif

#if MY_INDEX == 9

// 100k GE Sensing AL03006-58.2K-97-G1 (4.7k pullup)

const static PROGMEM prog_uchar MY_NAME[] = "9. GE AL03006-58.2K";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 936},
    {   36 * TEMP_OVERSAMPLE, 300},
    {   71 * TEMP_OVERSAMPLE, 246},
    {  106 * TEMP_OVERSAMPLE, 218},
    {  141 * TEMP_OVERSAMPLE, 199},
    {  176 * TEMP_OVERSAMPLE, 185},
    {  211 * TEMP_OVERSAMPLE, 173},
    {  246 * TEMP_OVERSAMPLE, 163},
    {  281 * TEMP_OVERSAMPLE, 155},
    {  316 * TEMP_OVERSAMPLE, 147},
    {  351 * TEMP_OVERSAMPLE, 140},
    {  386 * TEMP_OVERSAMPLE, 134},
    {  421 * TEMP_OVERSAMPLE, 128},
    {  456 * TEMP_OVERSAMPLE, 122},
    {  491 * TEMP_OVERSAMPLE, 117},
    {  526 * TEMP_OVERSAMPLE, 112},
    {  561 * TEMP_OVERSAMPLE, 107},
    {  596 * TEMP_OVERSAMPLE, 102},
    {  631 * TEMP_OVERSAMPLE,  97},
    {  666 * TEMP_OVERSAMPLE,  92},
    {  701 * TEMP_OVERSAMPLE,  87},
    {  736 * TEMP_OVERSAMPLE,  81},
    {  771 * TEMP_OVERSAMPLE,  76},
    {  806 * TEMP_OVERSAMPLE,  70},
    {  841 * TEMP_OVERSAMPLE,  63},
    {  876 * TEMP_OVERSAMPLE,  56},
    {  911 * TEMP_OVERSAMPLE,  48},
    {  946 * TEMP_OVERSAMPLE,  38},
    {  981 * TEMP_OVERSAMPLE,  23},
    { 1005 * TEMP_OVERSAMPLE,   5},
    { 1016 * TEMP_OVERSAMPLE,   0}
};

#endif

#if MY_INDEX == 10

const static PROGMEM prog_uchar MY_NAME[] = "10. RS 198-961";

static const Entry MY_TABLE[] PROGMEM = {
    {    1 * TEMP_OVERSAMPLE, 929},
    {   36 * TEMP_OVERSAMPLE, 299},
    {   71 * TEMP_OVERSAMPLE, 246},
    {  106 * TEMP_OVERSAMPLE, 217},
    {  141 * TEMP_OVERSAMPLE, 198},
    {  176 * TEMP_OVERSAMPLE, 184},
    {  211 * TEMP_OVERSAMPLE, 173},
    {  246 * TEMP_OVERSAMPLE, 163},
    {  281 * TEMP_OVERSAMPLE, 154},
    {  316 * TEMP_OVERSAMPLE, 147},
    {  351 * TEMP_OVERSAMPLE, 140},
    {  386 * TEMP_OVERSAMPLE, 134},
    {  421 * TEMP_OVERSAMPLE, 128},
    {  456 * TEMP_OVERSAMPLE, 122},
    {  491 * TEMP_OVERSAMPLE, 117},
    {  526 * TEMP_OVERSAMPLE, 112},
    {  561 * TEMP_OVERSAMPLE, 107},
    {  596 * TEMP_OVERSAMPLE, 102},
    {  631 * TEMP_OVERSAMPLE,  97},
    {  666 * TEMP_OVERSAMPLE,  91},
    {  701 * TEMP_OVERSAMPLE,  86},
    {  736 * TEMP_OVERSAMPLE,  81},
    {  771 * TEMP_OVERSAMPLE,  76},
    {  806 * TEMP_OVERSAMPLE,  70},
    {  841 * TEMP_OVERSAMPLE,  63},
    {  876 * TEMP_OVERSAMPLE,  56},
    {  911 * TEMP_OVERSAMPLE,  48},
    {  946 * TEMP_OVERSAMPLE,  38},
    {  981 * TEMP_OVERSAMPLE,  23},
    { 1005 * TEMP_OVERSAMPLE,   5},
    { 1016 * TEMP_OVERSAMPLE,   0}
};

#endif

#if MY_INDEX == 11

const static PROGMEM prog_uchar MY_NAME[] = "11. QWG-104-3950";
    {    1 * TEMP_OVERSAMPLE, 938},
    {   31 * TEMP_OVERSAMPLE, 314},
    {   41 * TEMP_OVERSAMPLE, 290},
    {   51 * TEMP_OVERSAMPLE, 272},
    {   61 * TEMP_OVERSAMPLE, 258},
    {   71 * TEMP_OVERSAMPLE, 247},
    {   81 * TEMP_OVERSAMPLE, 237},
    {   91 * TEMP_OVERSAMPLE, 229},
    {  101 * TEMP_OVERSAMPLE, 221},
    {  111 * TEMP_OVERSAMPLE, 215},
    {  121 * TEMP_OVERSAMPLE, 209},
    {  131 * TEMP_OVERSAMPLE, 204},
    {  141 * TEMP_OVERSAMPLE, 199},
    {  151 * TEMP_OVERSAMPLE, 195},
    {  161 * TEMP_OVERSAMPLE, 190},
    {  171 * TEMP_OVERSAMPLE, 187},
    {  181 * TEMP_OVERSAMPLE, 183},
    {  191 * TEMP_OVERSAMPLE, 179},
    {  201 * TEMP_OVERSAMPLE, 176},
    {  221 * TEMP_OVERSAMPLE, 170},
    {  241 * TEMP_OVERSAMPLE, 165},
    {  261 * TEMP_OVERSAMPLE, 160},
    {  281 * TEMP_OVERSAMPLE, 155},
    {  301 * TEMP_OVERSAMPLE, 150},
    {  331 * TEMP_OVERSAMPLE, 144},
    {  361 * TEMP_OVERSAMPLE, 139},
    {  391 * TEMP_OVERSAMPLE, 133},
    {  421 * TEMP_OVERSAMPLE, 128},
    {  451 * TEMP_OVERSAMPLE, 123},
    {  491 * TEMP_OVERSAMPLE, 117},
    {  531 * TEMP_OVERSAMPLE, 111},
    {  571 * TEMP_OVERSAMPLE, 105},
    {  611 * TEMP_OVERSAMPLE, 100},
    {  641 * TEMP_OVERSAMPLE,  95},
    {  681 * TEMP_OVERSAMPLE,  90},
    {  711 * TEMP_OVERSAMPLE,  85},
    {  751 * TEMP_OVERSAMPLE,  79},
    {  791 * TEMP_OVERSAMPLE,  72},
    {  811 * TEMP_OVERSAMPLE,  69},
    {  831 * TEMP_OVERSAMPLE,  65},
    {  871 * TEMP_OVERSAMPLE,  57},
    {  881 * TEMP_OVERSAMPLE,  55},
    {  901 * TEMP_OVERSAMPLE,  51},
    {  921 * TEMP_OVERSAMPLE,  45},
    {  941 * TEMP_OVERSAMPLE,  39},
    {  971 * TEMP_OVERSAMPLE,  28},
    {  981 * TEMP_OVERSAMPLE,  23},
    {  991 * TEMP_OVERSAMPLE,  17},
    { 1001 * TEMP_OVERSAMPLE,   9},
    { 1021 * TEMP_OVERSAMPLE, -27}
};

#endif

#endif // BOARD_TYPE_AZTEEG_X3

#endif // THERMISTOR_TABLES_H_
