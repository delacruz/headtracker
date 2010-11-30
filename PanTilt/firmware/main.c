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
#include "Servo.h"
#include "RCInput.h" 
#include "headtracker_defs.h"
#include "crc.h"
#include "Delay.h"

#define ON 1
#define OFF 0
#define MODEM_COMMAND_MODE(state) (state ? (PORTC |= (1<<1)) : (PORTC &= ~(1<<1)))

volatile uint16_t gMeasuredPulseWidth = 1500;
char gNewCommandAvailable = 0;
char gBadHappened = 0;
uint16_t gCrcErrorCountUplink = 0;
uint16_t gRejectedFrames = 0;

char Sync(unsigned char* buffer, unsigned char* index);
char HasKnownPacketAvailable(uint8_t *buffer, uint8_t* index);
void HandleHeadtrackerCommand(unsigned char* uplinkFrame, uint8_t* uplinkFrameIndexPtr, uint16_t* targetPanServoPulsePtr, uint16_t* targetTiltServoPulsePtr);
void SendDownlinkPacket(void);
uint8_t GetSignalReport(void);

int main(void)
{
	// Uplink & Downlink buffers
	unsigned char uplinkBuffer[255];
	unsigned char uplinkBufferIndex=0;

	// Servo pulse widths
	uint16_t initialPanServoPulse = 1500;
	uint16_t initialTiltServoPulse = 1500;
	uint16_t targetPanServoPulse = 1500;
	uint16_t targetTiltServoPulse = 1500;
	uint16_t smoothedPanServoPulse = 1500;
	uint16_t smoothedTiltServoPulse = 1500;
	
	// Tick vars for servo command interpolation
	tick_t tickLastValidUplinkPacket = 0;
	tick_t ticksBetweenPackets = 0;
	tick_t interpolationTick = 0;
	
	char isFrameInSync = 0;
	char resetInterpolation = 0;
	
	// Initialize Hardware
	InitHardware();
	
	// Set 1st pin of PORTC as output, used for CFG pin of Maxstream Xtend
	DDRC |= (1<<0);		// Set as output
	PORTC &= ~(1<<0);	// Set to off
	
	// Set pin 2 of PORTC as output, used for SHDN sinal, must be high for xtend radio modem to be on
	DDRC |= (1<<1);		// Set as output
	PORTC &= ~(1<<1);	// Set to off
	
	InitServoTimer(3);
	
	// Open UART
    fdevopen( UART0_PutCharStdio, UART0_GetCharStdio );
	
	/* 
	// Configure for 115200 baud
	MODEM_COMMAND_MODE(ON);
	UART0_PutChar(0x15);
	UART0_PutChar(0x07);
	UART0_PutChar(0x08);
	Delay10uSec(12);
	MODEM_COMMAND_MODE(OFF);
	 */
	
	// Main Loop
    while(1)
	{
		// Get all available bytes from UART0
		while(UART0_IsCharAvailable()) uplinkBuffer[uplinkBufferIndex++] = UART0_GetChar();
		
		// Sync if necessary
		if (!isFrameInSync) isFrameInSync = Sync(uplinkBuffer, &uplinkBufferIndex);

		
		// Process all packets in buffer
		while (isFrameInSync && HasKnownPacketAvailable(uplinkBuffer, &uplinkBufferIndex))
		{

			uint8_t packetType = uplinkBuffer[HEADER_SIZE];
			
			// Handle based on packet type
			switch (packetType) 
			{
				case FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND:
					
					// Handle headtracker servo commands
					HandleHeadtrackerCommand(uplinkBuffer, &uplinkBufferIndex, &targetPanServoPulse, &targetTiltServoPulse);
					
					// New servo commands, reset interpolation
					resetInterpolation = 1;
					
					// Done
					break;
						
					// Unknown packet type, must be out of sync
				default:
					// TODO: CODE WILL NEVER REACH THIS POINT, CONDITION IS CAPTURED BY HasFullPacketAvailable()
					isFrameInSync = 0;
					break;
			}
		} // End process all packets in buffer
		
		// Process 50Hz Activities
		if(gTickCount%2==0)
		{
			
			// Blink blue light if we received a valid packet
			if (gNewCommandAvailable) 
			{
				LED_ON(BLUE);
				gNewCommandAvailable=0;
			}
			else 
			{
				LED_OFF(BLUE);
			}
		}
		
		// Process 5Hz Activities
		if(gTickCount%20==0) // 5Hz
		{
			// Blink red LED anytime bad happens
			if (gBadHappened) 
			{
				LED_ON(RED);
				gBadHappened = 0;
			}
			else 
			{
				LED_OFF(RED);
			}
		}
		
		// Process 2Hz Activities
		if(gTickCount%50==0) // 2Hz
		{
			SendDownlinkPacket();
			LED_TOGGLE(YELLOW);
		}
		
		if (gTickCount%200==0) {
			
		}
		
		
		
		// If new servo command has been processed, reset interpolation
		if (resetInterpolation && gTickCount != tickLastValidUplinkPacket) 
		{
			// Get interpolation time span
			ticksBetweenPackets = gTickCount - tickLastValidUplinkPacket;
			
			// Set variable for last valid packet to now
			tickLastValidUplinkPacket = gTickCount;
			
			// Initial servo positions become = to wherever they are now
			initialPanServoPulse = smoothedPanServoPulse;
			initialTiltServoPulse = smoothedTiltServoPulse;
			
			// Reset our interpolation tick
			interpolationTick = 0;
			
			resetInterpolation = 0;
		}	
		
		// If either servo has not reached it's target pulse, continue interpolation
		if (smoothedPanServoPulse != targetPanServoPulse || smoothedTiltServoPulse!=targetTiltServoPulse) 
		{
			interpolationTick++;
			smoothedPanServoPulse = (uint16_t)(((int)targetPanServoPulse-(int)initialPanServoPulse) * (interpolationTick / (float)ticksBetweenPackets) + (int)initialPanServoPulse);
			smoothedTiltServoPulse = (uint16_t)(((int)targetTiltServoPulse-(int)initialTiltServoPulse) * (interpolationTick / (float)ticksBetweenPackets) + (int)initialTiltServoPulse);
		}

		// Set servos
		SetServo(SERVO_3A, smoothedPanServoPulse);
		SetServo(SERVO_3B, smoothedTiltServoPulse);
	
		WaitForTimer0Rollover();
    }
	
	// You've gone too far
    return 0; 
}

char HasKnownPacketAvailable(uint8_t *buffer, uint8_t* index)
{
	// Must have at least a header and packet type
	if (*index < HEADER_SIZE + 1) 
	{
		return 0;
	}
	
	// Get the packet type
	uint8_t packetType = buffer[HEADER_SIZE];
	
	// 
	switch (packetType) {
		case FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND:
			
			return *index >= FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND_SIZE;
			
			break;
			
		default:
			
			// Unknown frame type, must discard.  Chop head.
			memcpy(buffer, &buffer[HEADER_SIZE], *index-HEADER_SIZE);
			*index -= HEADER_SIZE;
			
			// Re-sync
			Sync(buffer, index);
			
			// Increase rejected frame count
			gRejectedFrames++;
			
			return 0;
			break;
	}
}

char Sync(unsigned char* buffer, unsigned char* index)
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
			return 1;
		}
	}
	
	// No header found, set index to 0
	*index = 0;
	return 0;
}

void HandleHeadtrackerCommand(unsigned char* uplinkFrame, uint8_t* uplinkFrameIndexPtr, uint16_t* targetPanServoPulsePtr, uint16_t* targetTiltServoPulsePtr)
{
	if(crc16_verify(uplinkFrame, FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND_SIZE)) 
	{
		
		// Set flag
		gNewCommandAvailable = 1;
		
		// Handle Packet
		uint8_t *ptr = (uint8_t*)uplinkFrame;
		
		// Skip Header
		ptr+=2;
		
		// Skip packet-type id
		ptr+=1;
		
		// Grab pan servo pulse
		memcpy(targetPanServoPulsePtr, ptr, sizeof(*targetPanServoPulsePtr));
		ptr += sizeof(*targetPanServoPulsePtr);
		
		// Grab tilt servo pulse
		memcpy(targetTiltServoPulsePtr, ptr, sizeof(*targetTiltServoPulsePtr));
		ptr += sizeof(*targetTiltServoPulsePtr);
		
		//printf("\nTarget Pan Servo Pulse: %hu\tTarget Tilt Servo Pulse: %hu", *targetPanServoPulsePtr, *targetTiltServoPulsePtr);
		
		// Pop the packet
		memcpy(uplinkFrame, &uplinkFrame[FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND_SIZE], *uplinkFrameIndexPtr-FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND_SIZE);
		*uplinkFrameIndexPtr-=FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND_SIZE;
		
	}
	else 
	{
		// Update crc error counter
		gCrcErrorCountUplink++;
		gBadHappened = 1;
		
		// Chop off head
		memcpy(uplinkFrame, &uplinkFrame[2], *uplinkFrameIndexPtr-2);
		*uplinkFrameIndexPtr-=2;
		
		// Trim bad bytes til next header
		Sync(uplinkFrame, uplinkFrameIndexPtr);
		
		
	}
	
}

void PulseDetected( uint8_t channel, uint16_t pulseWidth )
{
	gMeasuredPulseWidth = pulseWidth;
	
	LED_TOGGLE( BLUE );
}

void MissingPulse( void )
{
	//gLastChannel = 0;
	gMeasuredPulseWidth = 6969;
	LED_TOGGLE( YELLOW );
}

void SendDownlinkPacket()
{
	uint8_t downlinkFrame[255];
	unsigned char downlinkFrameIndex=0;
	
	uint16_t header = 0xbeef;
	
	memcpy(downlinkFrame, &header, sizeof(header));
	downlinkFrameIndex += sizeof(header);
	
	memcpy(&downlinkFrame[downlinkFrameIndex], &gCrcErrorCountUplink, sizeof(gCrcErrorCountUplink));
	downlinkFrameIndex += sizeof(gCrcErrorCountUplink);
	
	downlinkFrame[downlinkFrameIndex] = GetSignalReport();
	downlinkFrameIndex++;
	
	uint16_t crc = 0xffff;
	crc = crc16_array_update(&downlinkFrame[2], UPLINK_REPORT_PACKET_SIZE-HEADER_SIZE-CRC_SIZE);
	
	memcpy(&downlinkFrame[downlinkFrameIndex], &crc, sizeof(crc));
	downlinkFrameIndex += sizeof(crc);
	
	//	int i = 0;
	//	printf("\n DOWNLINK FRAME: ");
	//	for (i=0; i<downlinkFrameIndex; i++) {
	//		printf("%2X ", downlinkFrame[i]);
	//	}
	
	UART0_Write(downlinkFrame, UPLINK_REPORT_PACKET_SIZE);
}

uint8_t GetSignalReport(void)
{
	// Enter Xtend modem's binary command mode
	MODEM_COMMAND_MODE(ON);
	
	// Query Signal strength: 0x36 | 0x80 = 0xB6
	UART0_PutChar(0xB6);
	
	// Read LSB
	uint8_t lsb = UART0_GetChar();
	
	// Read MSB
	uint8_t msb = UART0_GetChar();
	
	// Minimum 100uSec wait after command as per page 18 Xtend OEM manual
	// TODO: May not be necessary as being waited on by GetChar()
	Delay100uSec(2);
	
	// Exit Xtend modem's binary command mode
	MODEM_COMMAND_MODE(OFF);
	
	// Xtend return 0x8000 when not yet sampled, otherwise returns
	//  value between 40 and 110, unsigned.
	return (msb == 0x80) ? 0xFF : lsb;
}
