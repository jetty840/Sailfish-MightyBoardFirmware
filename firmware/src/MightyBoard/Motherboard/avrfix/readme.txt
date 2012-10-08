=============================================================================
Readme for Fixed Point Library for Atmel 8 bit Processors
=============================================================================
     __ ____  _________
    /_ \\\  \/   /|    \______
   //   \\\     /|| D  /_    /.
  //     \\\_  /.||    \ ___/.
 /___/\___\\__/. |__|\__\__.___  ___ 
  ....  .......   ...||  _/_\  \////. 
                     || |.| |\  ///.
Content:             |__|.|_|///  \
- About               .... ./__/\__\
- Requirements               ........
- Files
- Compiling
- Usage


-----------------------------------------------------------------------------
About
-----------------------------------------------------------------------------

This library is dedicated for use with Atmel 8 bit Processors and is designed
according to ISO/IEC paper DTR 18037.
For Details please use the documents in the 'docu' directory.


-----------------------------------------------------------------------------
Requirements
-----------------------------------------------------------------------------

- gcc
- AVRtools is required for compiling and linking
- ar for building the library
- make, if the makefile is being used


-----------------------------------------------------------------------------
Files
-----------------------------------------------------------------------------

All Files of the library itself are in the directory named 'avrfix'. Those
Files are:
avrfix.c		implementation of the library
avrfix.h		headerfile of the library
avrfix_config.h		headerfile for configuring the library
Makefile		to make the library
sizes.txt		a list of function sizes
srecsize.c		used to get the codesize of functions
lgpl.txt		the licence of this project


-----------------------------------------------------------------------------
Compiling
-----------------------------------------------------------------------------

To use the library, it must be compiled first. It is recommended to use the
included Makefile, although it may be edited before it can be used.
First, the target architecture must be defined:

# target architecture
MCU	= atmega16

By default, the target architecture is the atmega16.

Also the path to some tools is needed:

# Tools
CC	= avr-gcc
AR = ar
AS	= avr-as
ASLD	= avr-gcc -x assembler
LD	= avr-ld
OBJCOPY = avr-objcopy
SRECSIZE = ./srecsize

After editing the Makefile, the library can be compiled by simply calling
'make'. The library is then compiled and archived into the file 'libavrfix.a',
if the value of PROJNAME is not changed in the Makefile.


------------------------------------------------------------------------------
Usage
------------------------------------------------------------------------------

To use the library, only the headerfile and the library itself is needed.
When compiling a project that uses the library, the library file needs to be
linked with the following options:

 -l avrfix -Wl,-Map=<projectname>.map,-L=<path to library> -mmcu=<target architecture>