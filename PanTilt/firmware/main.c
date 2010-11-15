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
	unsigned char downlinkFrame[255];
	unsigned char downlinkFrameIndex=0;
	uint16_t panServoPulse = 1500;
	uint16_t tiltServoPulse = 1500;
	uint16_t targetPanServoPulse = 1500;
	uint16_t targetTiltServoPulse = 1500;
	uint16_t initialPanServoPulse = 1500;
	uint16_t initialTiltServoPulse = 1500;
	
	tick_t tickLastValidUplinkPacket;
	tick_t ticksBetweenPackets;
	tick_t smoothingTick = 0;
	
	char newCommandAvailable = 0;
	char badHappened = 0;
	
	
	uint16_t crcErrorCountUplink = 0;
	
	InitHardware();
	InitServoTimer(3);
	InitServoTimer(1);
	
    u0 = fdevopen( UART0_PutCharStdio, UART0_GetCharStdio );
	
	tickLastValidUplinkPacket = gTickCount; // TODO: Necessary? Probly not.
	
    while(1)
	{
		while(UART0_IsCharAvailable())
		{
			uplinkFrame[uplinkFrameIndex++] = UART0_GetChar();
			
		}
		

		// Handle all available packets in Rx buffer
		while(uplinkFrameIndex >= UPLINK_PACKET_SIZE)
		{
			
			//printf("\nmore than one packet!\n");
			if(crc16_verify(&uplinkFrame, UPLINK_PACKET_SIZE)) // TODO: Add size of frame to systemwide include file
			{
				// Set flag
				newCommandAvailable = 1;
				
				// Handle Packet
				uint8_t *ptr = (uint8_t*)uplinkFrame;
				
				// Skip Header
				ptr+=2;
				
				// Grab pan servo pulse
				memcpy(&targetPanServoPulse, ptr, sizeof(targetPanServoPulse));
				ptr += sizeof(targetPanServoPulse);
				
				// Grab tilt servo pulse
				memcpy(&targetTiltServoPulse, ptr, sizeof(targetTiltServoPulse));
				ptr += sizeof(targetTiltServoPulse);
				
				//printf("\nPan Servo Pulse: %hu\tTilt Servo Pulse: %hu", panServoPulse, tiltServoPulse);
				
				// Pop the packet
				memcpy(uplinkFrame, &uplinkFrame[UPLINK_PACKET_SIZE], uplinkFrameIndex-UPLINK_PACKET_SIZE);
				uplinkFrameIndex-=UPLINK_PACKET_SIZE;
				
				if (gTickCount != tickLastValidUplinkPacket) 
				{
					ticksBetweenPackets = gTickCount - tickLastValidUplinkPacket;
					tickLastValidUplinkPacket = gTickCount;
					initialPanServoPulse = panServoPulse;
					initialTiltServoPulse = tiltServoPulse;
					smoothingTick = 0;
				}
				
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
			
			if (newCommandAvailable) 
			{
				LED_ON(BLUE);
				newCommandAvailable=0;
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
			downlinkFrameIndex = 0;
			
			memcpy(downlinkFrame, &header, sizeof(header));
			downlinkFrameIndex += sizeof(header);
			
			memcpy(&downlinkFrame[downlinkFrameIndex], &crcErrorCountUplink, sizeof(crcErrorCountUplink));
			downlinkFrameIndex += sizeof(crcErrorCountUplink);
			
			uint16_t crc = 0xffff;
			crc = crc16_array_update(&downlinkFrame[2], 2);//_crc16_update(crc, crcErrorCountUplink);
			
			memcpy(&downlinkFrame[downlinkFrameIndex], &crc, sizeof(crc));
			downlinkFrameIndex += sizeof(crc);
			
			//UART0_Write(downlinkFrame, UPLINK_REPORT_PACKET_SIZE);
			printf("\npan: %hu target: %hu ticks-between: %hu initial: %hu", panServoPulse, targetPanServoPulse, ticksBetweenPackets, initialPanServoPulse);
			LED_TOGGLE(YELLOW);
		}
		

		if (panServoPulse != targetPanServoPulse || tiltServoPulse!=targetTiltServoPulse) 
		{
			smoothingTick++;
			panServoPulse = (uint16_t)(((int)targetPanServoPulse-(int)initialPanServoPulse) * (smoothingTick / (float)ticksBetweenPackets) + (int)initialPanServoPulse);
			tiltServoPulse = (uint16_t)(((int)targetTiltServoPulse-(int)initialTiltServoPulse) * (smoothingTick / (float)ticksBetweenPackets) + (int)initialTiltServoPulse);
		}
		
		if (panServoPulse>2000) 
		{
			printf("\nBEEF: pan: %hu target-tick: %hu ticks-between: %hu initial: %hu smoothing tick: %hu", panServoPulse, targetPanServoPulse, ticksBetweenPackets, initialPanServoPulse, smoothingTick);
		}
		//tiltServoPulse = (uint16_t)((targetTiltServoPulse-initialTiltServoPulse) * (smoothingTick / (float)ticksBetweenPackets) + initialTiltServoPulse);
		//}
		// Set servos
		SetServo(SERVO_3A, panServoPulse);
		SetServo(SERVO_3B, tiltServoPulse);
	
		WaitForTimer0Rollover();

    }
    return 0;   /* never reached */
}
