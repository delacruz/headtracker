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
#include <util/crc16.h>

//extern int 

int main(void)
{
	FILE   *u0;
    FILE   *u1;
	unsigned char uplinkFrame[255];
	unsigned char bufferIn[255];
	unsigned char bufferIndex=0;
	
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
		
		while(UART0_IsCharAvailable())
		{
			bufferIn[bufferIndex++] = UART0_GetChar();;
		}

	
		if(gTickCount%5==0)  // 20Hz
		{
			LED_TOGGLE(RED);
			//UART0_PutCharStdio(0x4a,u0);

		}
		
		if(gTickCount%10==0) // 10Hz
		{
			//LED_TOGGLE(BLUE);
		}
		
		if(gTickCount%20==0) // 5Hz
		{
			
		}
		
		if(gTickCount%50==0) // 2Hz
		{
			LED_TOGGLE(YELLOW);
			
			uint8_t testBuffer[] = {0x06,0x03,0x07,0x07,0x06,0x03,0xef,0xbe,0x07,0x07,0x06,0x03,0xef,0xbe,0x19,0x12,0x34,0xff};
			
			uint16_t crc = 0xffff;
		
			int i;
			
			for(i=0;i<sizeof(testBuffer);i++)
			{
				crc = _crc16_update(crc, testBuffer[i]);
			}

			UART0_Write(testBuffer, 18);

		}
		
		WaitForTimer0Rollover();
    }
    return 0;   /* never reached */
}
