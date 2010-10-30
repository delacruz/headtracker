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
*   @file    a2d_8.c
*
*   @brief   Contains code for retrieving 8 bit A/D samples.
*
*****************************************************************************/

/* ---- Include Files ----------------------------------------------------- */

#include <avr/io.h>
#include "Config.h"
#include "Hardware.h"
#include "a2d.h"

/* ---- Public Variables -------------------------------------------------- */
/* ---- Private Constants and Types --------------------------------------- */
/* ---- Private Variables ------------------------------------------------- */
/* ---- Private Function Prototypes --------------------------------------- */
/* ---- Functions --------------------------------------------------------- */

/****************************************************************************/
/**
*   Reads an 8 bit A/D value.
*/

#if CFG_USE_ADC

uint8_t a2d_8( uint8_t Channel )
{
    // Select the channel in a manner which leaves REFS0 and REFS1 un touched.

    ADMUX = ( ADMUX & (( 1 << REFS1 ) | ( 1 << REFS0 ))) | ( 1 << ADLAR ) | Channel;

    // Start the conversion
    ADCSR = ADCSR | ( 1 << ADSC );

    // Wait for it to complete
    while ( ADCSR & ( 1 << ADSC ));

    // We only need the top 8 bits (left-adjusted)
    return ADCH;

} // a2d _8

#endif  // CFG_USE_ADC

