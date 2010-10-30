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
*   @file   Servo.c
*
*   @brief  Defines an interface for manipulating servos
*
****************************************************************************/

// ---- Include Files -------------------------------------------------------

#include <avr/io.h>
#include "Config.h"
#include "Servo.h"
#include "Timer.h"

// ---- Public Variables ----------------------------------------------------
// ---- Private Constants and Types -----------------------------------------

#if   defined (__AVR_ATmega8__) \
   || defined (__AVR_ATmega48__) \
   || defined (__AVR_ATmega88__) \
   || defined (__AVR_ATmega168__)

#define INIT_1_DDR  DDRB |= (( 1 << 1 ) | ( 1 << 2 ))

#elif defined (__AVR_ATmega16__) \
   || defined (__AVR_ATmega32__) \
   || defined (__AVR_ATmega644__)

#define INIT_1_DDR  DDRD |= (( 1 << 4 ) | ( 1 << 5 ))

#elif defined (__AVR_ATmega64__) \
   || defined (__AVR_ATmega128__)

#define INIT_1_DDR  DDRB |= (( 1 << 5 ) | ( 1 << 6 ) | ( 1 << 7 ))
#define INIT_3_DDR  DDRE |= (( 1 << 3 ) | ( 1 << 4 ) | ( 1 << 5 ))

#else
#   error   Common/Servo.c Processor not supported
#endif

// ---- Private Function Prototypes -----------------------------------------
// ---- Functions -----------------------------------------------------------

/***************************************************************************/
/**
*  Initialize a timer used for Hardware PWM to drive a servo
*/

void InitServoTimer( uint8_t timerNum )
{
#if ( CFG_CPU_CLOCK != 16000000L )
#   error   InitServoTimer requires CFG_CPU_CLOCK set to 16 MHz
#endif

    // We run the timer at 2 MHz (divde by 8 prescalar), which means that
    // each tick of the timer corresponds to 1/2 usec.
    //
    // We use WGM mode 14, Fast PWM with the TOP value supplied by ICRx.
    // We use COM mode 2, which causes the OCxy pin to be cleared on a match
    // with OCRxy and set when the timer reaches TOP.
    //
    // With TOP set to 40,000, we get 40,000 1/2 usec counts or about 20 msec
    // between pulses (50 Hz) which is a suitable pulse rate for R/C servos.

    if ( timerNum == 1 )
    {
        ICR1 = 40000u;
        TCNT1 = 0;

        // Set the WGM mode & prescalar

        TCCR1A = ( 1 << WGM11 ) | ( 0 << WGM10 ) | ( 1 << COM1A1 ) | ( 1 << COM1B1 )
#if defined( COM1C1 )
            | ( 1 << COM1C1 )
#endif
            ;

        TCCR1B = ( 1 << WGM13 )  | ( 1 << WGM12 ) | TIMER1_CLOCK_SEL_DIV_8;

        INIT_1_DDR;
    }
#if defined( TCCR3A )
    else
    if ( timerNum == 3 )
    {
        ICR3 = 40000u;
        TCNT3 = 0;

        // Set the WGM mode & prescalar

        TCCR3A = ( 1 << WGM31 ) | ( 0 << WGM30 ) | ( 1 << COM3A1 ) | ( 1 << COM3B1 ) | ( 1 << COM3C1 );
        TCCR3B = ( 1 << WGM33 )  | ( 1 << WGM32 ) | TIMER3_CLOCK_SEL_DIV_8;

        INIT_3_DDR;
    }
#endif

} // InitServoTimer

/***************************************************************************/
/**
*  Sets the servo position
*/

void SetServo( uint8_t servoNum, uint16_t pulseWidthUSec )
{
    uint16_t    pulseWidthTicks = pulseWidthUSec * 2;    // Convert to ticks;

    switch ( servoNum )
    {
        case SERVO_1A:  OCR1A = pulseWidthTicks;    break;
        case SERVO_1B:  OCR1B = pulseWidthTicks;    break;
#if defined( OCR1C )
        case SERVO_1C:  OCR1C = pulseWidthTicks;    break;
#endif
#if defined( OCR3A )
        case SERVO_3A:  OCR3A = pulseWidthTicks;    break;
        case SERVO_3B:  OCR3B = pulseWidthTicks;    break;
        case SERVO_3C:  OCR3C = pulseWidthTicks;    break;
#endif
    }

} // SetServo
