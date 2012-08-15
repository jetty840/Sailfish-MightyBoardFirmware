# Initialization Procedure for lcd module using 4bit mode  

## Data Format Explanation

for all commands using writeSerial, the strobe pin is pulsed high then low after clocking out the 8 data bytes  

data is loaded in nibbles.  Thus there are two load procedures for each byte.  
each load clocks 8 bits (upper 4 bits are data, lower 4 bytes are mode / function bits)  

data format  
[data3, data2, data1, data0, enable, DC, command/data, DC]  
DC (don't care) bits are not changed in our interface and should always be zero    

4 bit load procedure (do this twice for 8 bit load)  
[d3, d2, d1, d0, 0, 0, M, 0] clock 8 bits, enable bit low   
[d3, d2, d1, d0, 1, 0, M, 0] clock 8 bits, enable bit high  
[d3, d2, d1, d0, 0, 0, M, 0] clock 8 bits, enable bit low  

M is command/data bit   

the strobe pin is pulled high, then pulled low after each clock of 8 bits  

In the following C(--) indicates a command byte and D(--) indicates a data byte.  These are 8 bit loads    
4bit (--) indicates a 4 bit load procedure    

## Initialization Sequence

// First we pull both RS and R/W low to begin commands    
  4bit (0x0)  
  
// Now we send display function command 4 times  
  C(0x28)   
  // wait 4.5ms  
  C(0x28)  
  // wait 150us  
  C(0x28)  
  C(0x28)    
  
// set #lines, blinking on off, cursor on /off  
  C(0x0C)  

// clear display  
  C(0x01)  

// set entry mode  
  C(0x05)  

// define special characters  
  C(0x40)  
  D(0), D(4), D(2), D(1), D(2), D(4), D(0)  
  C(0x40)  
  D(0), D(4), D(2), D(1), D(2), D(4), D(0)  
  C(0x48)  
  D(0), D(0), D(0), D(0), D(0), D(0x11), D(0xA), D(4)   
  C(0x48)  
  D(0), D(0), D(0), D(0), D(0), D(0x11), D(0xA), D(4)   


// this completes the initialization sequence.  Data after this is to load characters to the screen.  

