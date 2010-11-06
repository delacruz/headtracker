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
#include <string.h>

//extern int 

int main(void)
{
	FILE   *u0;
    FILE   *u1;
	//char bufferIn[255];
	//int bufferIndex=0;
	
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
		
		if(UART0_IsCharAvailable())
		{
			char c = UART0_GetCharStdio(u0);
			
			printf("Received %x\n",c);

//			bufferIn[bufferIndex++] = c;
//			int i;
//			for (i = 0; i< bufferIndex; i++) 
//			{
//				printf("%x ", bufferIn[i]);
//			}
//			printf("\n");
			LED_TOGGLE(BLUE);
		}
	
		if(gTickCount%5==0)  // 20Hz
		{
			LED_TOGGLE(RED);
			//UART0_PutCharStdio(0x4a,u0);

		}
		
		if(gTickCount%10==0) // 10Hz
		{

		}
		
		if(gTickCount%20==0) // 5Hz
		{
			
		}
		
		if(gTickCount%50==0) // 2Hz
		{
			LED_TOGGLE(YELLOW);
			unsigned short test = 0xBEEF;
			unsigned char buffer[2];
			memcpy(&buffer, &test, sizeof(short));
			UART0_Write(buffer, sizeof(short));
		}
		
		WaitForTimer0Rollover();
    }
    return 0;   /* never reached */
}
