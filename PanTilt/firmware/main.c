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

#define UPLINK_PACKET_SIZE 8
#define UPLINK_REPORT_PACKET_SIZE 6

void scoot(unsigned char* buffer, unsigned char* index);
void scoot(unsigned char* buffer, unsigned char* index)
{
	// Discard bytes until a header is found
	int i;
	for(i=0; i+1<*index; i++)  // i+1 is needed as the header is 2 bytes
	{
		if (buffer[i]==0xef && buffer[i+1]==0xbe) 
		{
			if (i != 0)  // Frame header not at start of frame
			{
				// Move from header ownwards to start of frame
				memcpy(buffer, &buffer[i], *index-i);
				
				// Adjust the index
				*index -= i;
			}
			
			// Done here, header at start of frame
			break;
		}
	}
}

uint16_t crc16_array_update(uint8_t array, uint8_t length);
uint16_t crc16_array_update(uint8_t array, uint8_t length)
{
	uint16_t crc = 0xffff;
	while (length>0)
	{
		crc = _crc16_update(crc, array++);
		length--;
	}
	
	return crc;
}

char crc16_verify(void* array, uint8_t length);
char crc16_verify(void* array, uint8_t length)
{
	uint16_t crc = 0xffff;
	
	uint8_t *ptr = (uint8_t *)array;
	
	// Skip header
	ptr+=2;  // TODO: define HEADER_SIZE somewhere, and include
	
	length-=4; // TODO: define CRC_SIZE somewhere, and include
	
	crc = crc16_array_update(*ptr, length);
	
	uint16_t packet_crc;
	memcpy(&packet_crc, ptr+length, 2);  // length=length of payload
	
	return crc == packet_crc;
}

int main(void)
{
	FILE   *u0;
    //FILE   *u1;
	unsigned char uplinkFrame[255];
	unsigned char uplinkFrameIndex=0;
	unsigned char uplinkReportFrame[255];
	unsigned char uplinkReportFrameIndex=0;
	
	uint16_t crcErrorsUplink = 0;
	
	InitHardware();
	
    u0 = fdevopen( UART0_PutCharStdio, UART0_GetCharStdio );

    //char c;
	
    while(1)
	{
		
		int newStuff = 0;
		
		while(UART0_IsCharAvailable())
		{
			uplinkFrame[uplinkFrameIndex++] = UART0_GetChar();
			newStuff = 1;
		}
		
		if (newStuff==1) 
		{
//			printf("\n Before Scoot: ");
//			int i;
//			for (i=0; i<uplinkFrameIndex; i++) 
//			{
//				printf("%.2X ", uplinkFrame[i]);
//			}
			
			scoot(uplinkFrame, &uplinkFrameIndex);
			
//			printf("\n After Scoot: ");
//			
//			for (i=0; i<uplinkFrameIndex; i++) 
//			{
//				printf("%.2X ", uplinkFrame[i]);
//			}
			
			
			while(uplinkFrameIndex >= UPLINK_PACKET_SIZE)
			{
				//printf("\nmore than one packet!\n");
				if(crc16_verify(&uplinkFrame, UPLINK_PACKET_SIZE)) // TODO: Add size of frame to systemwide include file
				{
					// Handle Packet
					uplinkFrameIndex-=UPLINK_PACKET_SIZE;;
				}
				else 
				{
					// Update crc error counter
					crcErrorsUplink++;
					
					// Pop the packet
					memcpy(uplinkFrame, &uplinkFrame[UPLINK_PACKET_SIZE], uplinkFrameIndex-UPLINK_PACKET_SIZE);
					uplinkFrameIndex-=UPLINK_PACKET_SIZE;
				}
				
				newStuff=0;
				
				LED_TOGGLE(BLUE);

			}

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
			
			uint8_t testBuffer[] = {0x06,0x03,0x07,0x07,0x06,0x03,0xef,0xbe,0x07,0x07,0x06,0x03,0xef,0xbe,0x19,0x12,0x34,0xff};
			
			uint16_t crc = 0xffff;
		
			int i;
			
			for(i=0;i<sizeof(testBuffer);i++)
			{
				crc = _crc16_update(crc, testBuffer[i]);
			}
			
			printf("\nCRC BAD COUNT: %d", crcErrorsUplink);
			
			uint16_t header = 0xbeef;
			
			memcpy(uplinkReportFrame, &header, sizeof(header));
			uplinkReportFrameIndex += sizeof(header);
			
			memcpy(uplinkReportFrame, &crcErrorsUplink, sizeof(crcErrorsUplink));
			uplinkReportFrameIndex += sizeof(crcErrorsUplink);
			
			uint16_t crcUplinkReport = 0xffff;
			crcUplinkReport = _crc16_update(crc, crcErrorsUplink);
			
			memcpy(uplinkReportFrame, &crcUplinkReport, sizeof(crcUplinkReport));
			uplinkReportFrameIndex += sizeof(crcUplinkReport);
			
			UART0_Write(uplinkReportFrame, UPLINK_REPORT_PACKET_SIZE);

		}
		
		WaitForTimer0Rollover();
    }
    return 0;   /* never reached */
}
