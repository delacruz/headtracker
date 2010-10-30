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

#include <avr/io.h>
#include <stdio.h>

#include "a2d.h"
#include "Hardware.h"
#include "Delay.h"
#include "Servo.h"
#include "Timer.h"
#include "UART.h"
#include "string.h"

uint8_t gADC[ 8 ];

int main(void)
{
    int     i;
    //char   *spinner = "/-\\|";
    //int     spinnerIdx = 0;
	char graph[200] = "";

    InitHardware();

    // The first handle opened for read goes to stdin, and the first handle
    // opened for write goes to stdout.

#if defined( __AVR_LIBC_VERSION__ )
    fdevopen( UART0_PutCharStdio, UART0_GetCharStdio );
#else
    fdevopen( UART0_PutCharStdio, UART0_GetCharStdio, 0 );
#endif

    printf( "\n\n*****\n" );
    printf( "***** Simple-Servo-2 program\n" );
    printf( "*****\n\n" );

	//getchar();

    InitServoTimer( 1 );
	InitServoTimer( 3 );

	uint16_t pulseMin = 1000u;
	uint16_t pulseMax = 2000u;
	uint16_t pulse_usec = pulseMax;
	uint16_t val = 100u;

	SetServo( SERVO_1A, 1500 );
	SetServo( SERVO_1B, 1500 );
	SetServo( SERVO_1C, 1500 );
    
	SetServo( SERVO_3A, 1500 );
    SetServo( SERVO_3B, 1500 );
	SetServo( SERVO_3C, 1500 );
	
	LED_TOGGLE( YELLOW );

    while( 1 )
    {
        //uint16_t    pulse1_usec;
        //uint16_t    pulse2_usec;

        // Read the ADC's

        gADC[ 0 ] = a2d_8( 0 );
        //gADC[ 1 ] = a2d_8( 1 );

        // "nominal" servo pulses are between 1 ms and 2 ms (1000 usec and 2000 usec)
        //  Some servos have a wider range, so I used 500usec thru 2500 usec range
        // and the user can use this to find the limits.
        //
        // The "nominal" formula would be:
        //
        //  pulse (in usec) = ( adc / 255 ) * (pulse_max - pulse_min) + pulse_min;
        //
        // Since we're using integer arithmetic, we need to rearrange things:
        //
        //  pulse = ( adc * 2000 ) / 255 + 500
        //
        // However, adc * 2000 exceeds 16 bits. 2000 / 255 is close enough to 8
        // for our purposes, so we rewrite it to get:
        //
        //  pulse = ( adc * 8 ) + 500
        //
        // The actual range of this formula is 500 - 2540
        //
        // We then need to multiply the pulse in usec by 2, since out timer 
        // runs with 1/2 usec ticks.

        //pulse1_usec = ( (uint16_t)gADC[ 0 ] * 8u ) + 500u;
        //pulse2_usec = ( (uint16_t)gADC[ 1 ] * 8u ) + 500u;

        //SetServo( SERVO_3A, pulse1_usec );
        //SetServo( SERVO_3B, pulse2_usec );

        // We put a delay in here so we aren't trying to update the OCR 
        // registers too frequently. Waiting to 2 ticks coincides with the
        // pulse rate.

		
		
		//pulse2_usec = ( (uint16_t)gADC[ 1 ] * 8u ) + 500u;

        SetServo( SERVO_3A, pulse_usec );
		SetServo( SERVO_3B, pulse_usec );
		SetServo( SERVO_3C, pulse_usec );
		
		SetServo( SERVO_1A, pulse_usec );
		SetServo( SERVO_1B, pulse_usec );
		SetServo( SERVO_1C, pulse_usec );
		
		
		
		if(pulse_usec == pulseMin || pulse_usec == pulseMax)
			val = val * -1u;
			
		pulse_usec += val;
		
		LED_TOGGLE( BLUE );
		
		
		
		
        for ( i = 0; i < 2; i++ ) 
        {
            // Toggle our heartbeat LED and update the display 4 times a second.
            // gTickCount increments every 10 msec

            WaitForTimer0Rollover();

			
			
			
            if (( gTickCount % 25 ) == 0 )
            {
                // The main loop runs once every 30 msec, and we toggle every 8th
                // time through, so we toggle about 4 times/sec.

                LED_TOGGLE( RED );
					
				for(i=0; i<(int)gADC[ 0 ]; i+=2)
					strcat(graph, "*");

				printf( "Pulse: %d\tADC: %d\tGraph: %s\n", pulse_usec,gADC[ 0 ],graph);

				memset(graph,0x0,sizeof(graph));

                //printf( "%c Servo1 adc: %3d pulse: %4d  Servo2 adc: %3d pulse: %4d    \r",
                //        spinner[ spinnerIdx ], gADC[ 0 ], pulse1_usec, gADC[ 1 ], pulse2_usec );

                //spinnerIdx++;
                //spinnerIdx &= 0x03;
            }
        }
		
    }

    return 0;
}

