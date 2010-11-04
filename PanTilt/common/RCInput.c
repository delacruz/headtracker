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
*   @file   RCInput.c
*
*   @brief  This file implements a way of measuring multiple channels from
*           an RC Receiver.
*
****************************************************************************/

// ---- Include Files -------------------------------------------------------

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "Config.h"
#include "RCInput.h"
#include "Timer.h"

// ---- Public Variables ----------------------------------------------------
// ---- Private Constants and Types -----------------------------------------

#if !defined( CFG_RCI_TIMER )
#   error CFG_RCI_TIMER must be defined in Config.h
#endif

//---------------------------------------------------------------------------
#if ( CFG_RCI_TIMER == 1 )

#define RCI_OVF_vect    TIMER1_OVF_vect
#define RCI_CAPT_vect   TIMER1_CAPT_vect
#define RCI_TCNT        TCNT1

//---------------------------------------------------------------------------
#elif ( CFG_RCI_TIMER == 2 )

#if !defined( CFG_RCI_INT_NUM )
#   error Timer 2 needs to have CFG_RCI_INT_NUM defined to be 0-7
#endif

#define RCI_OVF_vect                TIMER2_OVF_vect
#define RCI_TCNT                    GetTimer2Count()

#define EXT_INT_VECT_(x)            INT ## x ## _vect
#define EXT_INT_VECT(x)             EXT_INT_VECT_(x)
#define RCI_CAPT_vect               EXT_INT_VECT(CFG_RCI_INT_NUM)

#if ( CFG_RCI_INT_NUM < 4 )
#   define RCI_EXT_INT_DDR          DDRD
#   define RCI_EXT_INT_EICR         EICRA
#   define RCI_EXT_INT_EICR_VAL     ( 0x3 << ( CFG_RCI_INT_NUM * 2 ))
#else
#   define RCI_EXT_INT_DDR          DDRE
#   define RCI_EXT_INT_EICR         EICRB
#   define RCI_EXT_INT_EICR_VAL     ( 0x3 << (( CFG_RCI_INT_NUM - 4 ) * 2 ))
#endif

#define RCI_EXT_INT_MASK            ( 1 << CFG_RCI_INT_NUM )

//---------------------------------------------------------------------------
#elif ( CFG_RCI_TIMER == 3 )

#define RCI_OVF_vect    TIMER3_OVF_vect
#define RCI_CAPT_vect   TIMER3_CAPT_vect
#define RCI_TCNT        TCNT3

//---------------------------------------------------------------------------
#else
#   error   Only values of 1, 2, and 3 are supported for CFG_RCI_TIMER
#endif

#if !defined( CFG_RCI_SYNC_TIME )
#define CFG_RCI_SYNC_TIME           3000
#endif

// ---- Private Variables ---------------------------------------------------

/**
 * gOverflowCount is used to detect the lack of pulses. We assume that the
 * counter is running at 2 MHz, which means that it will overflow once every
 * 32.768 mSec.
 * 
 * Since the overflow isn't synchronized with the pulses, we need the overflow
 * count to be 2 before we consider pulses to be missing. This means that 
 * it will take 32 to 65 msec to detect missing pulses.
 * 
 * The overflow counter is reset to zero every time a pulse is detected
 */

static  volatile    uint8_t     gOverflowCount = -1;

#if ( CFG_RCI_TIMER == 2 )

/**
 * Timer 2 is only an 8-bit counter, so gTimer2OverflowCount is 
 * used to make the upper 8-bits.
 */

static volatile uint8_t gTimer2OverflowCount;
#endif

/**
 * gPulseIndex is used to keep track of which pulse we're working with. 
 * gPulseIndex is set to 1 when the sync time is detected, and incremented 
 * on other edges.
 * 
 * If gPulseIndex is 0 then edges are ignored (because we've been missing
 * pulses or because we haven't seen the sync)
 */

static  volatile    uint8_t     gPulseIndex    = 0;

/**
 * gLastEdgeTime records the time that the last edge occurred.
 */

static  volatile    uint16_t    gLastEdgeTime  = 0;

/**
 * gPulseDetectedCB is called whenever a pulse has been detected.
 */

static  RCI_PulseDetectedCB     gPulseDetectedCB;

/**
 * gMissingPulseCB is called whenever a lack of pulses is detected.
 */

static  RCI_MissingPulseCB      gMissingPulseCB;

// ---- Private Function Prototypes -----------------------------------------
// ---- Functions -----------------------------------------------------------

#if ( CFG_RCI_TIMER == 2 )
/***************************************************************************/
/**
*   Combines TCNT and gTimer2OverflowCount to produce a 16-bit timer value.
*
*   This function assumes that its called with interrupts disabled.
*/

inline uint16_t GetTimer2Count( void );

inline uint16_t GetTimer2Count( void )
{
    uint8_t cntHi;
    uint8_t cntLo;

    cntLo = TCNT2;
    cntHi = gTimer2OverflowCount;
    if ( TIFR & ( 1 << TOV2 ))
    {
        // Timer 2 overflowed - Since interrupts are disabled, this means
        // that the overflow interrupt hasn't occurred yet, so we 
        // compensate here.
        
        cntLo = TCNT2;
        cntHi++;
    }

    return ((uint16_t)cntHi << 8 ) | (uint16_t)cntLo;

} // GetTimer2Count

#endif

/***************************************************************************/
/**
*   Determines if we're in the "missing pulse" state.
*/

inline uint8_t PulsesAreMissing( void );
inline uint8_t PulsesAreMissing( void )
{
    return gOverflowCount >= 2;

} // PulsesAreMissing

/***************************************************************************/
/**
*   Interrupt handler for the Timer overflow
*/

volatile uint32_t gXOverflowCount;

ISR( RCI_OVF_vect )
{
    gXOverflowCount++;

#if ( CFG_RCI_TIMER == 2 )
    {
        // The 8-bit timer overflowed.

        gTimer2OverflowCount++;

        if ( gTimer2OverflowCount != 0 )
        {
            return;
        }

        // When gTimer2OverflowCount becomes 0, we fall through, since 
        // we've just overflowed the 16-bit counter.
    }
#endif
    if ( PulsesAreMissing() )
    {
        gPulseIndex = 0;

        if ( gMissingPulseCB != NULL )
        {
            gMissingPulseCB();
        }
    }
    else
    {
        gOverflowCount++;
    }

} // RCI_OVF_vect

/***************************************************************************/
/**
*   Interrupt handler for the Capture event
*/

volatile uint32_t gCaptureCount = 0;

ISR( RCI_CAPT_vect )
{
    uint16_t    captureTime = RCI_TCNT;

    gCaptureCount++;

    if ( PulsesAreMissing() )
    {
        // The first pulse in a long time. We can't call it a sync pulse.

        gPulseIndex = 0;    // pulses will be ignored until we hit a sync
        gOverflowCount = 0; // We're not in an overflow condition any more

        gLastEdgeTime = captureTime;
    }
    else
    {
        uint16_t    pulseWidth = captureTime - gLastEdgeTime;

        gLastEdgeTime = captureTime;

        pulseWidth >>= 1;  // Divide by 2 to convert into usec

        if ( pulseWidth >= CFG_RCI_SYNC_TIME )
        {
            // We've detected a sync pulse. This means that we're now 
            // measuring the first pulse.

            gPulseIndex = 1;
            gOverflowCount = 0;
        }
        else
        {
            if ( gPulseDetectedCB != NULL )
            {
                gPulseDetectedCB( gPulseIndex, pulseWidth );
            }

            gPulseIndex++;
        }
    }

} // RCI_CAPT_vect

/***************************************************************************/
/**
*   Initializes the timer used for capturing the input stream.
*/

void RCI_Init( void )
{
    gOverflowCount = -1;
    gPulseIndex = 0;
    gLastEdgeTime = 0;

#if ( CFG_CPU_CLOCK == 16000000 )

    // Set the timer to run at 2 MHz (divide by 8 prescalar)

#   if ( CFG_RCI_TIMER == 1 )

    // Since the 3 outputs could be used for driving servos, we don't touch
    // any bits associated with controlling the output.

    // WGM Mode 0: Normal is also fine. We do use the overflow interrupt
    // and the ICR register so some modes aren't fine.

    TCCR1B = ( TCCR1B & ~TIMER1_CLOCK_SEL_MASK )
           | (( 1 << ICNC1 ) | ( 1 << ICES1 ) | TIMER1_CLOCK_SEL_DIV_8 );

    TCNT1H = 0; // Need to write high byte first
    TCNT1L = 0;

    TIMSK |= (( 1 << TICIE1 ) | ( 1 << TOIE1 ));

#   elif ( CFG_RCI_TIMER == 2 )

    // WGM Mode 0: Normal mode

    TCCR2 = TIMER2_CLOCK_SEL_DIV_8;
    gTimer2OverflowCount = 0;
    TCNT2 = 0;
    TIMSK |= ( 1 << TOIE2 );

    // Disable the external interrupt so we don't false while changing the EICR register

    EIMSK &= ~RCI_EXT_INT_MASK;

    // Configure the appropriate input pin as an input

    RCI_EXT_INT_DDR &= ~RCI_EXT_INT_MASK;

    // Configure the external interrupt as triggering on the rising edge.

    RCI_EXT_INT_EICR |= RCI_EXT_INT_EICR_VAL;

    // Re-eanble the external interrupt

    EIMSK |= RCI_EXT_INT_MASK;

#   elif ( CFG_RCI_TIMER == 3 )

    TCCR3B = ( TCCR3B & ~TIMER3_CLOCK_SEL_MASK ) 
           | (( 1 << ICNC3 ) | ( 1 << ICES3 ) | TIMER3_CLOCK_SEL_DIV_8 );

    TCNT3H = 0; // Need to write high byte first
    TCNT3L = 0;

    ETIMSK |= (( 1 << TICIE3 ) | ( 1 << TOIE3 ));

#   else
#       error Unsupported value for CFG_RCI_TIMER
#   endif
#else
#   error Unsupported clock frequency
#endif

} // RCI_Init

/***************************************************************************/
/**
*  Sets the callback which is called when a pulse is detected.
*/

void RCI_SetPulseCallback( RCI_PulseDetectedCB pulseDetectedCB )
{
    gPulseDetectedCB = pulseDetectedCB;

} // RCI_SetPulseCallback

/***************************************************************************/
/**
*  Sets the callback which is called when no pulses are detected.
*/

void RCI_SetMissingPulseCallback( RCI_MissingPulseCB missingPulseCB )
{
    gMissingPulseCB = missingPulseCB;

} // RCI_SetMissingPulseCallback

