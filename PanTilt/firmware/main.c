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
#include "crc16.h"
#include "Delay.h"
#include <avr/io.h>
#if defined( __AVR_LIBC_VERSION__ )
#   include <avr/interrupt.h>
#else
#include <avr/signal.h>
#endif
#include "uplink.h"
#include "downlink.h"

#define ON 1
#define OFF 0
#define MODEM_COMMAND_MODE(state) (state ? (PORTC |= (1<<1)) : (PORTC &= ~(1<<1)))

typedef struct 
{
	uint8_t signalStrength;
	uint8_t txPower;
	uint8_t boardVoltage;
	uint8_t boardTemperature;
} ModemReport;

volatile char gIsFailsafeModeOn		= 0;
volatile char gIsFailsafeSignalOk	= 0;

char gNewCommandAvailable			= 0;
char gBadHappened					= 0;
uint16_t gCrcErrorCountUplink		= 0;
uint16_t gRejectedFrames			= 0;

// Uplink & Downlink buffers
unsigned char gUplinkBuffer[255];
unsigned char gUplinkBufferIndex=0;

char Sync(unsigned char* buffer, unsigned char* index);
char HasKnownPacketAvailable(uint8_t *buffer, uint8_t* index);
void HandleHeadtrackerCommand(unsigned char* uplinkFrame, uint8_t* uplinkFrameIndexPtr, uint16_t* targetPanServoPulsePtr, uint16_t* targetTiltServoPulsePtr);
void HandleModemConfigCommand(unsigned char* uplinkFrame, uint8_t* uplinkFrameIndexPtr);
void SendDownlinkPacket(ModemReport* modemReport);
inline void QueryModemReport(ModemReport* modemReport);
inline void ReadAllAvailableUplinkBytes(void);
void PulseDetected(uint8_t channel, uint16_t pulseWidth);
void MissingPulse(void);

unsigned char gDownlinkFrameCount = 0;

int main(void)
{
	// Servo pulse widths
	uint16_t initialPanServoPulse	= 1500;
	uint16_t initialTiltServoPulse	= 1500;
	uint16_t targetPanServoPulse	= 1500;
	uint16_t targetTiltServoPulse	= 1500;
	uint16_t smoothedPanServoPulse	= 1500;
	uint16_t smoothedTiltServoPulse	= 1500;
	uint16_t videoRelayPulse		= 1000;
	
	// Tick vars for servo command interpolation
	tick_t tickLastValidUplinkPacket	= 0;
	tick_t ticksBetweenPackets			= 0;
	tick_t interpolationTick			= 0;
	
	char isFrameInSync		= 0;
	char resetInterpolation	= 0;
	
	ModemReport modemReport;
	
	// Initialize Hardware
	InitHardware();
	
	// Set 1st pin of PORTC as output.........
	DDRC |= (1<<0);		// Set as output
	PORTC &= ~(1<<0);	// Set to off
	
	// Set pin 2 of PORTC as output, used for bin command sinal, must be high for xtend radio modem to be on
	DDRC |= (1<<1);		// Set as output
	PORTC &= ~(1<<1);	// Set to off
	
	InitServoTimer(1);
	
	SetServo(SERVO_1A, initialPanServoPulse);
	SetServo(SERVO_1B, initialTiltServoPulse);
	SetServo(SERVO_1C, videoRelayPulse);
	
	RCI_Init();
	sei();
	
	RCI_SetPulseCallback(PulseDetected);
	RCI_SetMissingPulseCallback(MissingPulse);
	
	// Open UART
    fdevopen( UART0_PutCharStdio, UART0_GetCharStdio );
	
	// Main Loop
    while(1)
	{
		ReadAllAvailableUplinkBytes();
		
		// Sync if necessary
		if (!isFrameInSync) isFrameInSync = Sync(gUplinkBuffer, &gUplinkBufferIndex);

		
		// Process all packets in buffer
		while (isFrameInSync && HasKnownPacketAvailable(gUplinkBuffer, &gUplinkBufferIndex))
		{

			uint8_t packetType = gUplinkBuffer[sizeof(delim_t)];
			
			// Handle based on packet type
			switch (packetType) 
			{
					
				case KIND_HEADTRACKER_CMD:
					
					// Handle headtracker servo commands
					HandleHeadtrackerCommand(gUplinkBuffer, &gUplinkBufferIndex, &targetPanServoPulse, &targetTiltServoPulse);
					
					// New servo commands, reset interpolation
					resetInterpolation = 1;
					
					// Done
					break;
					
				case KIND_MODEM_CMD:
						
					//  Set Modem Configuration
					HandleModemConfigCommand(gUplinkBuffer, &gUplinkBufferIndex);
					
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
		
		if (gTickCount%200==0) // Query Modem Report every 2 seconds
		{
			QueryModemReport(&modemReport);
		}
		
		// Process 2Hz Activities
		if(gTickCount%50==0) // 2Hz
		{
			
			SendDownlinkPacket(&modemReport);
			LED_TOGGLE(YELLOW);
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
				
		videoRelayPulse = gIsFailsafeModeOn ? 2000 : 1000;

		// Set servos
		SetServo(SERVO_1A, smoothedPanServoPulse);
		SetServo(SERVO_1B, smoothedTiltServoPulse);
		SetServo(SERVO_1C, videoRelayPulse);
	
		WaitForTimer0Rollover();
    }
	
	// You've gone too far
    return 0; 
}

char HasKnownPacketAvailable(uint8_t *buffer, uint8_t* index)
{
	// Must have at least a header
	if (*index < sizeof(header_t)) 
	{
		return 0;
	}
	
	// Get the packet type
	uint8_t packetType = buffer[sizeof(delim_t)];
	
	// 
	switch (packetType) {
		case KIND_HEADTRACKER_CMD:
			
			return *index >= sizeof(headtracker_cmd_t_pkt);
			
			break;
			
		case KIND_MODEM_CMD:
			
			return *index >= sizeof(modem_cmd_t_pkt);
			
			break;

		default:
			
			// Unknown frame type, must discard.  Chop head.
			memcpy(buffer, &buffer[sizeof(delim_t)], *index-sizeof(delim_t));
			*index -= sizeof(delim_t);
			
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
	if (*index!=1)  // Could be half a header
	{
		*index = 0;
	}
	
	return 0;
}

void HandleHeadtrackerCommand(unsigned char* uplinkFrame, uint8_t* uplinkFrameIndexPtr, uint16_t* targetPanServoPulsePtr, uint16_t* targetTiltServoPulsePtr)
{
	headtracker_cmd_t_pkt pkt;
	memcpy(&pkt, uplinkFrame, sizeof(headtracker_cmd_t_pkt));
	char isOk = verify_crc_headtracker_cmd_t_pkt(pkt);
	
	if(isOk)
	{
		gNewCommandAvailable = 1;
		
		*targetPanServoPulsePtr = pkt.message.pan_servo_pulse;
		*targetTiltServoPulsePtr = pkt.message.tilt_servo_pulse;	
	}
	else 
	{
		gBadHappened = 1;
		gCrcErrorCountUplink++;
	}
	
	// Pop the packet
	memcpy(uplinkFrame, &uplinkFrame[sizeof(headtracker_cmd_t_pkt)], *uplinkFrameIndexPtr-sizeof(headtracker_cmd_t_pkt));
	*uplinkFrameIndexPtr-=sizeof(headtracker_cmd_t_pkt);

}

void PulseDetected( uint8_t channel, uint16_t pulseWidth )
{
	gIsFailsafeSignalOk = 1;
	gIsFailsafeModeOn = (pulseWidth>1500) ? 0 : 1;
	
	LED_TOGGLE(BLUE);
	
}

void MissingPulse( void )
{
	gIsFailsafeSignalOk = 0;
}

void SendDownlinkPacket(ModemReport* modemReport)
{
	downlink_t msg;
	
	msg.bad_uplink_crc_cnt = gCrcErrorCountUplink;
	msg.signal_strength = modemReport->signalStrength;
	msg.tx_power_level = modemReport->txPower;
	msg.is_video_relay_on = 1;//gIsFailsafeModeOn;
	msg.is_failsafe_mode = gIsFailsafeModeOn;
	msg.is_failsafe_com_ok = gIsFailsafeSignalOk;
	msg.counter = gDownlinkFrameCount++;
	
	downlink_t_pkt pkt = create_downlink_t_pkt(msg);
	
	UART0_Write(&pkt, sizeof(pkt));

}

inline void QueryModemReport(ModemReport* modemReport)
{	
	// Enter Xtend modem's binary command mode
	MODEM_COMMAND_MODE(ON);
	
	// Query Signal strength: 0x36 | 0x80 = 0xB6
	UART0_PutChar(0xB6);
	
	// Read LSB
	uint8_t lsb = UART0_GetChar();
	
	// Read MSB
	uint8_t msb = UART0_GetChar();
	
	// Xtend return 0x8000 when not yet sampled, otherwise returns
	// value between 40 and 110, unsigned; this code will return 0xff
	// if not yet sampled.
	modemReport->signalStrength = (msb == 0x80) ? 0xFF : lsb;
	
	// Query Tx Power Level
	UART0_PutChar(0x3A | 0x80);
	// No delay here causes byte to sometimes xmit outside of command mode,
	// delay probably allows time for buffer to raise interrupt
	// and uart to pick tx the byte
	Delay100uSec(3);
	
	// Read Tx Power Level
	modemReport->txPower = UART0_GetChar();
	
	// Exit Xtend modem's binary command mode
	MODEM_COMMAND_MODE(OFF);
}

inline void ReadAllAvailableUplinkBytes(void)
{
	// Get all available bytes from UART0
	while(UART0_IsCharAvailable()) gUplinkBuffer[gUplinkBufferIndex++] = UART0_GetChar();
}

void HandleModemConfigCommand(unsigned char* uplinkFrame, uint8_t* uplinkFrameIndexPtr)
{
	modem_cmd_t_pkt pkt;
	memcpy(&pkt, uplinkFrame, sizeof(modem_cmd_t_pkt));
	
	char isOk = verify_crc_modem_cmd_t_pkt(pkt);
	
	if (isOk) 
	{
		gNewCommandAvailable = 1;
		
		MODEM_COMMAND_MODE(ON);
		
		UART0_PutChar(0x3A);
		UART0_PutChar(pkt.message.tx_power_level);
		UART0_PutChar(0x00);
		
		MODEM_COMMAND_MODE(OFF);
	}
	else 
	{
		gBadHappened = 1;
	}
	
	memcpy(uplinkFrame, &uplinkFrame[sizeof(modem_cmd_t_pkt)], *uplinkFrameIndexPtr-sizeof(modem_cmd_t_pkt));
	*uplinkFrameIndexPtr-=sizeof(modem_cmd_t_pkt);

}
