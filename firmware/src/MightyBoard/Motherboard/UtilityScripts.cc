/*
 * Copyright 2012 by Alison Leonard <alison@makerbot.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
 
 #include "UtilityScripts.hh"
 #include <avr/pgmspace.h>
 
 static PROGMEM int Lengths[2]  = { 404,  /// FilamentRight
							100}; /// FilamentLeft

 
 static PROGMEM uint8_t FilamentRight[] = { 149,  0,  0,  0,  0,  70,  105,  108,  97,  109,  101,  
	 110,  116,  32,  76,  111,  97,  100,  32,  83,  99,  114,  105,  112,  116,  
	 0,  149,  3,  0,  0,  0,  32,  80,  114,  101,  115,  115,  32,  77,  32,  
	 116,  111,  32,  67,  111,  110,  116,  105,  110,  117,  101,  0,  148,  
	 255,  200,  0,  1,  149,  0,  0,  0,  5,  72,  101,  97,  116,  105,  110,  
	 103,  32,  102,  111,  114,  32,  32,  32,  32,  32,  32,  32,  32,  32,  0,  
	 149,  3,  0,  0,  5,  32,  32,  32,  102,  105,  108,  97,  109,  101,  110,  
	 116,  32,  114,  101,  108,  101,  97,  115,  101,  0,  137,  8,  153,  0,  0,  
	 0,  0,  82,  101,  112,  71,  32,  66,  117,  105,  108,  100,  0,  150,  0,  
	 255,  136,  0,  3,  2,  225,  0,  134,  0,  135,  0,  100,  0,  255,  255,  
	 149,  2,  0,  0,  90,  69,  106,  101,  99,  116,  105,  110,  103,  32,  70,  
	 105,  108,  97,  109,  101,  110,  116,  0,  137,  136,  136,  0,  10,  1,  3,  
	 136,  0,  4,  1,  255,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  
	 64,  56,  0,  0,  0,  0,  0,  0,  128,  74,  93,  5,  31,  149,  0,  0,  0, 
	  0,  82,  101,  109,  111,  118,  101,  32,  111,  108,  100,  32,  70,  105,  
	  108,  97,  109,  101,  110,  116,  32,  0,  149,  1,  0,  0,  0,  76,  111,  
	  97,  100,  32,  110,  101,  119,  32,  70,  105,  108,  97,  109,  101,  110,  
	  116,  32,  32,  32,  0,  149,  3,  0,  0,  0,  80,  114,  101,  115,  115, 
	   32,  77,  32,  116,  111,  32,  67,  111,  110,  116,  105,  110,  117, 
		101,  0,  148,  255,  200,  0,  1,  137,  136,  149,  2,  0,  0,  120,  
		76,  111,  97,  100,  105,  110,  103,  32,  70,  105,  108,  97,  109, 
		 101,  110,  116,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
		  0,  0,  181,  255,  255,  0,  0,  0,  0,  0,  14,  39,  7,  31,  149, 
		   0,  0,  0,  5,  76,  111,  97,  100,  105,  110,  103,  32,  83,  99, 
			114,  105,  112,  116,  32,  32,  32,  32,  32,  32,  0,  149,  3,  0,
			  0,  5,  32,  32,  32,  67,  111,  109,  112,  108,  101,  116,  101,
				0,  137,  8,  136,  0,  3,  2,  0,  0,  150,  100,  255,  154, 
				 0,  136,  0,  10,  1,  2,  136,  0,  4,  1,  255,  137,  31 };

 
 namespace utility {
	 
	 volatile bool is_playing;
	 int build_index = 0;
	 int build_length = 0;
	 uint8_t * buildFile;
	 

	 
 
 /// returns true if script is running
 bool isPlaying(){
	 return is_playing;		
 }
 
 /// returns true if more bytes are available in the script
 bool playbackHasNext(){
	return (build_index < build_length);
 }
 
 /// gets next byte in script
 uint8_t playbackNext(){
	 
	 if(build_index < build_length)
		return buildFile[build_index++];

	else 
		return 0;
 }
 
 /// begin buffer playback
 bool startPlayback(uint8_t build){

	switch (build){
		case FILAMENT_RIGHT:
			buildFile = FilamentRight;
			build_length = Lengths[FILAMENT_RIGHT];
			is_playing = true;
			break;
		default:
			is_playing = false;
	}
	
	 build_index = 0;
	 return is_playing;
 }
 
 /// updates state to finished playback
 void finishPlayback(){
 
	is_playing = false;
 }

};

