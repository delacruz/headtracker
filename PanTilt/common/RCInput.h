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
*   @file   RCInput.h
*
*   @brief  This file implements a way of measuring multiple channels from
*           an RC Receiver.
*
*           These 3 PDF documents:
*           http://www.omegaco.demon.co.uk/mectnpdf/mectn003.pdf
*           http://www.omegaco.demon.co.uk/mectnpdf/mectn004.pdf
*           http://www.omegaco.demon.co.uk/mectnpdf/mectn005.pdf
*           provide some really good background information on how
*           radio controlled receivers work.
*
*           In particular, this code assumes that you can tap into the
*           clock signal, which is signal A in figure 29.
*
*           The particular RC receiver that this code was tested with was a
*           Hitec HAS-2MB Super Narrow Band 2 channel AM Receiver operating
*           in the 75 MHz band.
*
*           The receiver has exactly one IC one it, an HEF40175B,
*           which is a Quadruple D-type flip-flop.
*           http://www.semiconductors.philips.com/pip/HEF40175B.html
*           
*           The clock is triggered on the rising edge.
*
*           This code measures the time between consecutive rising edges,
*           which defines the pulse width for each channel. A difference of
*           3 msec or more is used to synchronize to channel 1.
*
*           This software will also detect the abscence of pulses, which 
*           requires 30-60 msec to go by with no pulses being detected.
*
*
*   Configuration Options
*
*           CFG_RCI_TIMER   Set to 1 or 3 to indicate which timer should
*                           be used.
*
*           CFG_RCI_SYNC_TIMER  specifies the minimum number of microseconds
*                               used to detect the sync period.
*
****************************************************************************/

#if !defined( RCINPUT_H )
#define RCINPUT_H                              ///< Include Guard

// ---- Include Files -------------------------------------------------------

#include <stdint.h>

// ---- Constants and Types -------------------------------------------------

/**
 *  The RCI_PulseDetectedCB describes a pointer to a function which will
 *  be called when a pulse is detected. channel will be the channel number
 *  which starts at 1, and pulseWidth will be the width of the pulse measured
 *  in microseconds.
 * 
 *  NOTE: This function is called from interrupt context.
 */

typedef void (*RCI_PulseDetectedCB)( uint8_t channel, uint16_t pulseWidth );

/**
 *  The RCI_MissingPulseCB describes a pointer to a function which will be
 *  called when a period of time passes and no pulses are detected.
 * 
 *  NOTE: This function is called from interrupt context.
 */

typedef void (*RCI_MissingPulseCB)( void );

// ---- Variable Externs ----------------------------------------------------

// ---- Function Prototypes -------------------------------------------------

void RCI_Init( void );
void RCI_SetPulseCallback( RCI_PulseDetectedCB pulseDetectedCB );
void RCI_SetMissingPulseCallback( RCI_MissingPulseCB missingPulseCB );

#endif  // RCINPUT_H

