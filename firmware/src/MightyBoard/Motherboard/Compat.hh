#ifndef __COMPAT_HH__
#define __COMPAT_HH__

#ifndef SIMULATOR

#include <avr/version.h>

#if __AVR_LIBC_VERSION__ >= 10800UL
#define __DELAY_BACKWARD_COMPATIBLE__ 1
#define __PROG_TYPES_COMPAT__ 1
#endif

#endif

#endif
