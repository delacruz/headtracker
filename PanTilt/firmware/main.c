/* Name: main.c
 * Author: Jason de la Cruz
 * Copyright: Mourning Wood Software, LLC.
 * License: <insert your license reference here>
 */

#include <avr/io.h>
#include <stdio.h>
#include "Hardware.h"
#include "Timer.h"
#include "UART.h"

int main(void)
{
	FILE   *u0;
    FILE   *u1;
	
	InitHardware();
	
#if defined( __AVR_LIBC_VERSION__ )
    u0 = fdevopen( UART0_PutCharStdio, UART0_GetCharStdio );
    u1 = fdevopen( UART1_PutCharStdio, NULL );
#else
    //u0 = fdevopen( UART0_PutCharStdio, UART0_GetCharStdio, 0 );
    //u1 = fdevopen( UART1_PutCharStdio, NULL, 0 );
#endif
    //char c;
	
    for(;;){
	
		if(gTickCount%5==0)  // 20Hz
		{
			LED_TOGGLE(RED);
		}
		
		if(gTickCount%20==0) // 5Hz
		{
			LED_TOGGLE(BLUE);
		}
		
		if(gTickCount%50==0) // 2Hz
		{
			LED_TOGGLE(YELLOW);
		}
		
		WaitForTimer0Rollover();
    }
    return 0;   /* never reached */
}
