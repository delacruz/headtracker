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
*   @file   Servo.h
*
*   @brief  Defines an interface for manipulating servos
*
****************************************************************************/

#if !defined( SERVO_H )
#define SERVO_H             /**< Include Guard                             */

/* ---- Include Files ---------------------------------------------------- */

#include <stdint.h>

/* ---- Constants and Types ---------------------------------------------- */

#define SERVO_1A    0
#define SERVO_1B    1
#define SERVO_1C    2

#define SERVO_3A    3
#define SERVO_3B    4
#define SERVO_3C    5

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

void    InitServoTimer( uint8_t timerNum );
void    SetServo( uint8_t servoNum, uint16_t pulseWidthUSec );
                  
#endif  // SERVO_H
