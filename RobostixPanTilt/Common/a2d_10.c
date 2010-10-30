/****************************************************************************
*
*   Copyright (c) 2007 Dave Hylands     <dhylands@gmail.com>
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
*   @file    a2d_10.c
*
*   @brief   Contains code for retrieving 10 bit A/D samples.
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
*   Reads a 10 bit A/D value.
*/

#if CFG_USE_ADC

uint16_t a2d_10( uint8_t Channel )
{
    // Select the channel in a manner which leaves REFS0 and REFS1 un touched.

    ADMUX = ( ADMUX & (( 1 << REFS1 ) | ( 1 << REFS0 ))) | Channel;

    // Start the conversion
    ADCSR = ADCSR | ( 1 << ADSC );

    // Wait for it to complete
    while ( ADCSR & ( 1 << ADSC ));

    return ADC;

} // a2d_10

#endif  // CFG_USE_ADC

