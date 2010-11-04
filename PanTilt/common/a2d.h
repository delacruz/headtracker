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
*   @file   Hardware.h
*
*   @brief  Defines all of the hardware definitions for the chip.
*
****************************************************************************/

#if !defined( A2D_H )
#define A2D_H                   /**< Include Guard                         */

/* ---- Include Files ---------------------------------------------------- */

#include <inttypes.h>
#include "Config.h"

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

#if CFG_USE_ADC

#if !defined( ADCSR )
#define ADCSR ADCSRA
#endif

uint8_t  a2d_8( uint8_t Channel );
uint16_t a2d_10( uint8_t Channel );

#endif  // CFG_USE_ADC
#endif // A2D_H

