
/*
 * Copyright (c) 2006-2010 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef SD_RAW_CONFIG_H
#define SD_RAW_CONFIG_H

#include <stdint.h>
#include "Configuration.hh"
#include "Pin.hh"

#define SD_TIMEOUT 1000  //1ms

// The Replicator 1, 2, and 2X use a 10 inch ribbon cable to connect between
// the SD card board and the main board.  Interwire capacitance on that cable
// is nearly 10 pF ( 9.2 pF measured ).  The SD card specification calls for
// a maximum bus capacitance of 10 pF.  So, it should come as no surprise that
// SD card comms on the Replicators is borderline.  Indeed, Dan has demonstrated
// SD cards which will work fine at lower SPI speeds but fail completely when
// run at max speed ( f_OSC / 2 ).

// Hence SD_POOR_DESIGN.  When set to a non-zero value, causes the SD card's
// SPI bus to run at f_OSC / 8.

#define SD_POOR_DESIGN 1

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup sd_raw
 *
 * @{
 */
/**
 * \file
 * MMC/SD support configuration (license: GPLv2 or LGPLv2.1)
 */

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write support.
 *
 * Set to 1 to enable MMC/SD write support, set to 0 to disable it.
 */
#define SD_RAW_WRITE_SUPPORT 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write buffering.
 *
 * Set to 1 to buffer write accesses, set to 0 to disable it.
 *
 * \note This option has no effect when SD_RAW_WRITE_SUPPORT is 0.
 */
#define SD_RAW_WRITE_BUFFERING 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD access buffering.
 * 
 * Set to 1 to save static RAM, but be aware that you will
 * lose performance.
 *
 * \note When SD_RAW_WRITE_SUPPORT is 1, SD_RAW_SAVE_RAM will
 *       be reset to 0.
 */
#define SD_RAW_SAVE_RAM 1

/**
 * \ingroup sd_raw_config
 * Controls support for SDHC cards.
 *
 * Set to 1 to support so-called SDHC memory cards, i.e. SD
 * cards with more than 2 gigabytes of memory.
 */
#ifndef SD_RAW_SDHC
#define SD_RAW_SDHC 0
#endif

/**
 * @}
 */

/* defines for customisation of sd/mmc port access */
#if defined(__AVR_ATmega8__) || \
    defined(__AVR_ATmega48__) || \
    defined(__AVR_ATmega48P__) || \
    defined(__AVR_ATmega88__) || \
    defined(__AVR_ATmega88P__) || \
    defined(__AVR_ATmega168__) || \
    defined(__AVR_ATmega168P__) || \
    defined(__AVR_ATmega328P__)
    #define configure_pin_mosi() DDRB |= (1 << DDB3)
    #define configure_pin_sck() DDRB |= (1 << DDB5)
    #define configure_pin_ss() DDRB |= (1 << DDB2)
    #define configure_pin_miso() DDRB &= ~(1 << DDB4)

    #define select_card() PORTB &= ~(1 << PORTB2)
    #define unselect_card() PORTB |= (1 << PORTB2)
#elif defined(__AVR_ATmega16__) || \
      defined(__AVR_ATmega32__) || \
      defined(__AVR_ATmega644P__)
    #define configure_pin_mosi() DDRB |= (1 << DDB5)
    #define configure_pin_sck() DDRB |= (1 << DDB7)
    #define configure_pin_ss() DDRB |= (1 << DDB4)
    #define configure_pin_miso() DDRB &= ~(1 << DDB6)

    #define select_card() PORTB &= ~(1 << PORTB4)
    #define unselect_card() PORTB |= (1 << PORTB4)
#elif defined(__AVR_ATmega64__) || \
      defined(__AVR_ATmega128__) || \
      defined(__AVR_ATmega169__) || \
	  defined(__AVR_ATmega1280__) || \
	  defined(__AVR_ATmega2560__)
    #define configure_pin_mosi() DDRB |= (1 << DDB2)
    #define configure_pin_sck() DDRB |= (1 << DDB1)
    #define configure_pin_ss() DDRB |= (1 << DDB0)
    #define configure_pin_miso() DDRB &= ~(1 << DDB3)

    #define select_card() PORTB &= ~(1 << PORTB0)
    #define unselect_card() PORTB |= (1 << PORTB0)
#else
    #error "no sd/mmc pin mapping available!"
#endif

#define configure_pin_available() SD_DETECT_PIN.setDirection(false)
#define configure_pin_locked() SD_WRITE_PIN.setDirection(false)

#define get_pin_available() SD_DETECT_PIN.getValue()
#define get_pin_locked() !SD_WRITE_PIN.getValue()

#if SD_RAW_SDHC
    typedef uint64_t offset_t;
#else
    typedef uint32_t offset_t;
#endif

/* configuration checks */
#if SD_RAW_WRITE_SUPPORT
#undef SD_RAW_SAVE_RAM
#define SD_RAW_SAVE_RAM 0
#else
#undef SD_RAW_WRITE_BUFFERING
#define SD_RAW_WRITE_BUFFERING 0
#endif

#ifdef __cplusplus
}
#endif

#endif

