#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define TIMER_FREQ 2000000

main()
{
  int i,j;
  int array[256][2];
  printf("#ifndef SPEED_LOOKUPTABLE_H\n");
  printf("#define SPEED_LOOKUPTABLE_H\n");
  printf("\n");    
  printf("#include <avr/pgmspace.h>\n");
  printf("\n");    
  printf("uint16_t speed_lookuptable_fast[256][2] PROGMEM = {\n");
  for(i=0;i<256;i++){
     array[i][0] = TIMER_FREQ / ((i*256)+32);
  }
  for(i=0;i<255;i++){
     array[i][1] = array[i][0] - array[i+1][0];
  }
  array[255][1] = array[254][1];
  for(i=0;i<32;i++) {
    for(j=0;j<8;j++) {
      printf("{ %d, %d}, ", array[i*8+j][0], array[i*8+j][1]);
    } 
    printf("\n");    
  }
  printf("};\n");    
  printf("uint16_t speed_lookuptable_slow[256][2] PROGMEM = {\n");
  for(i=0;i<256;i++){
     array[i][0] = TIMER_FREQ / ((i*8)+32);
  }
  for(i=0;i<255;i++){
     array[i][1] = array[i][0] - array[i+1][0];
  }
  array[255][1] = array[254][1];
  for(i=0;i<32;i++) {
    for(j=0;j<8;j++) {
      printf("{ %d, %d}, ", array[i*8+j][0], array[i*8+j][1]);
    } 
    printf("\n");    
  }
  printf("};\n");    
  printf("\n");
  printf("#endif\n");
}
