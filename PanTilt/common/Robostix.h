/****************************************************************************
*
*   Copyright (c) 2006 Dave Hylands     <dhylands@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation.
*
*   Alternatively, this software may be distributed under the terms of BSD
*   license.
*
*   See README and COPYING for more details.
*
****************************************************************************/
/**
*
*   @file   Robostix.h
*
*   @brief  Defines hardware features which are specific to the Robostix
*
****************************************************************************/

#if !defined( ROBOSTIX_H )
#define ROBOSTIX_H            /**< Include Guard                           */

/* ---- Include Files ---------------------------------------------------- */

#include <avr/io.h>

//--------------------------------------------------------------------------
// LED Constants

#define RED_LED_PIN     4
#define RED_LED_MASK    ( 1 << RED_LED_PIN )
#define RED_LED_DDR     DDRG
#define RED_LED_PORT    PORTG

#define BLUE_LED_PIN    3
#define BLUE_LED_MASK   ( 1 << BLUE_LED_PIN )
#define BLUE_LED_DDR    DDRG
#define BLUE_LED_PORT   PORTG

#define YELLOW_LED_PIN  4
#define YELLOW_LED_MASK ( 1 << YELLOW_LED_PIN )
#define YELLOW_LED_DDR  DDRB
#define YELLOW_LED_PORT PORTB

//--------------------------------------------------------------------------
// Some convenience macros to turn the LEDs on/off.
//
//  Usage:  LED_ON( BLUE );
//
// to turn on the blue LED.
//
// Note: Setting the pin to 0 turns the LED on.

#define LED_ON( color )     do { color ## _LED_PORT &= ~color ## _LED_MASK; } while (0)
#define LED_OFF( color )    do { color ## _LED_PORT |= color ## _LED_MASK; } while (0)
#define LED_TOGGLE( color ) do { color ## _LED_PORT ^= color ## _LED_MASK; } while (0)

#endif  // ROBOTSTIX_H

