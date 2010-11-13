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
#include "Servo.h"

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
			return;
		}
	}
	
	// No header found, set index to 0
	*index = 0;
}

uint16_t crc16_array_update(const void* array, uint8_t length);
uint16_t crc16_array_update(const void* array, uint8_t length)
{
	const uint8_t *ptr = (const uint8_t*)array;
	uint16_t crc = 0xffff;
	while (length>0)
	{
		crc = _crc16_update(crc, *ptr++);
		length--;
	}
	
	return crc;
}

char crc16_verify(const void* array, uint8_t length);
char crc16_verify(const void* array, uint8_t length)
{
	uint16_t crc = 0xffff;
	
	const uint8_t *ptr = (const uint8_t *)array;
	
	// Skip header
	ptr+=2;// TODO: define HEADER_SIZE somewhere, and include
	
	length-=4; // TODO: define CRC_SIZE somewhere, and include
	
	crc = crc16_array_update(ptr, length);
	
	uint16_t packet_crc = 0;
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
	uint16_t panServoPulse = 1500;
	uint16_t tiltServoPulse = 1500;
	char newStuff = 0;
	char badHappened = 0;
	
	uint16_t crcErrorCountUplink = 0;
	
	InitHardware();
	InitServoTimer(3);
	
    u0 = fdevopen( UART0_PutCharStdio, UART0_GetCharStdio );

    //char c;
	
    while(1)
	{
		while(UART0_IsCharAvailable())
		{
			uplinkFrame[uplinkFrameIndex++] = UART0_GetChar();
			
		}
		

		// Handle all available packets in Rx buffer
		while(uplinkFrameIndex >= UPLINK_PACKET_SIZE)
		{
			newStuff = 1;
			//printf("\nmore than one packet!\n");
			if(crc16_verify(&uplinkFrame, UPLINK_PACKET_SIZE)) // TODO: Add size of frame to systemwide include file
			{
				// Handle Packet
				uint8_t *ptr = (uint8_t*)uplinkFrame;
				
				// Skip Header
				ptr+=2;
				
				// Grab pan servo pulse
				memcpy(&panServoPulse, ptr, sizeof(panServoPulse));
				ptr += sizeof(panServoPulse);
				
				// Grab tilt servo pulse
				memcpy(&tiltServoPulse, ptr, sizeof(tiltServoPulse));
				ptr += sizeof(tiltServoPulse);
				
				//printf("\nPan Servo Pulse: %hu\tTilt Servo Pulse: %hu", panServoPulse, tiltServoPulse);
				
				// Pop the packet
				memcpy(uplinkFrame, &uplinkFrame[UPLINK_PACKET_SIZE], uplinkFrameIndex-UPLINK_PACKET_SIZE);
				uplinkFrameIndex-=UPLINK_PACKET_SIZE;
			}
			else 
			{
//				printf("\nBad CRC, for packet: ");
//				int i;
//				for (i=0; i<uplinkFrameIndex; i++) 
//				{
//					printf("%.2X ", uplinkFrame[i]);
//				}
//				
				
				// Update crc error counter
				crcErrorCountUplink++;
				badHappened = 1;
				
				// Chop off head
				memcpy(uplinkFrame, &uplinkFrame[2], uplinkFrameIndex-2);
				uplinkFrameIndex-=2;
				
//				printf("\nChopped head, now have: ");
//				for (i=0; i<uplinkFrameIndex; i++) 
//				{
//					printf("%.2X ", uplinkFrame[i]);
//				}
				
				// Trim bad bytes til next header
				scoot(uplinkFrame, &uplinkFrameIndex);
				
//				printf("\nNow trimmed: ");
//				for (i=0; i<uplinkFrameIndex; i++) 
//				{
//					printf("%.2X ", uplinkFrame[i]);
//				}
//				printf("\nTotal crc bad: %d", crcErrorCountUplink);
			}
				


		}
		
		if(gTickCount%2==0) // 50Hz
		{
			
			if (newStuff) 
			{
				LED_ON(BLUE);
				newStuff=0;
			}
			else 
			{
				LED_OFF(BLUE);
			}
			
			
			
		}
	
		if(gTickCount%5==0)  // 20Hz
		{


		}
		
		if(gTickCount%10==0) // 10Hz
		{



		}
		
		if(gTickCount%20==0) // 5Hz
		{
			if (badHappened) 
			{
				LED_ON(RED);
				badHappened = 0;
			}
			else 
			{
				LED_OFF(RED);
			}
		}
		
		if(gTickCount%50==0) // 2Hz
		{
			
			uint16_t header = 0xbeef;
			uplinkReportFrameIndex = 0;
			
			memcpy(uplinkReportFrame, &header, sizeof(header));
			uplinkReportFrameIndex += sizeof(header);
			
			memcpy(&uplinkReportFrame[uplinkReportFrameIndex], &crcErrorCountUplink, sizeof(crcErrorCountUplink));
			uplinkReportFrameIndex += sizeof(crcErrorCountUplink);
			
			uint16_t crc = 0xffff;
			crc = crc16_array_update(&uplinkReportFrame[2], 2);//_crc16_update(crc, crcErrorCountUplink);
			
			memcpy(&uplinkReportFrame[uplinkReportFrameIndex], &crc, sizeof(crc));
			uplinkReportFrameIndex += sizeof(crc);
			
			UART0_Write(uplinkReportFrame, UPLINK_REPORT_PACKET_SIZE);
			
			LED_TOGGLE(YELLOW);
		}
		
		// Set servos
		SetServo(SERVO_3A, panServoPulse);
		SetServo(SERVO_3B, tiltServoPulse);
	
		WaitForTimer0Rollover();
    }
    return 0;   /* never reached */
}
