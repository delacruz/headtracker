//
//  SerialReaderAppDelegate.m
//  SerialReader
//
//  Created by Jason de la Cruz on 8/31/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import "SerialReaderAppDelegate.h"
#import "crc16.h"
#import "serialreader.h"

#define HEADER_SIZE 2
#define CRC_SIZE 2
#define PAYLOAD_SIZE_HEADTRACKER 4
#define PACKET_SIZE_HEADTRACKER HEADER_SIZE + PAYLOAD_SIZE_HEADTRACKER + CRC_SIZE

@implementation SerialReaderAppDelegate

@synthesize window;

+ (int)openPort
{
	return open_port();
}

+ (int)openDownlinkPort
{
	return open_uplink_downlink_port();
}

+ (void)closePort: (int)withFileDescriptor
{
	
}

- (void)readerLoop
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	while (![[NSThread currentThread] isCancelled] && serialReadFileDescriptor != -1)
	{
		[self willChangeValueForKey:@"heading"];
		read_sensor_port(serialReadFileDescriptor, &heading, &pitch, &roll);
		[self didChangeValueForKey:@"heading"];
		
	}
	[pool drain];
}

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

#define PRINTRX 0

- (void)downlinkReaderLoop
{
	const int SIZE_FULL_FRAME = 6;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	unsigned char downlinkFrame[255];
	unsigned char buffer[255];
	unsigned char downlinkFrameIndex = 0;
	unsigned char numBytesRead;
	
	while (![[NSThread currentThread] isCancelled] && serialWriteFileDescriptor != -1)
	{
		// Get new bytes
		numBytesRead = read_downlink(serialWriteFileDescriptor, buffer);
		
#if PRINTRX
		
		for(int i=0; i<numBytesRead; i++)
		{
			printf("%c", buffer[i]);
		}
#else
		
		// Append to our downlink frame
		memcpy(&downlinkFrame[downlinkFrameIndex], buffer, numBytesRead);
		downlinkFrameIndex += numBytesRead;
		
		scoot(downlinkFrame, &downlinkFrameIndex);
		
		// Handle all packets in buffer
		while (downlinkFrameIndex >= SIZE_FULL_FRAME) 
		{
			// Check if packet is good
			if (crc16_verify(downlinkFrame, SIZE_FULL_FRAME)) 
			{
				// Update GUI
				NSString *bytesAsString = [[[NSString alloc] init] autorelease];
				for(int i=0; i<SIZE_FULL_FRAME; i++)
				{
					bytesAsString = [bytesAsString stringByAppendingFormat:@"%.2X ", downlinkFrame[i]];
				}
				[downlinkPacketFrameLabel setStringValue:bytesAsString];
				
				// Handle Packet
				unsigned char *ptr = downlinkFrame;
				
				// Skip Header
				ptr += 2;
				
				// Grab the reported uplink crc errors
				unsigned short uplinkCrcErrorCount;
				memcpy(&uplinkCrcErrorCount, ptr, sizeof(uplinkCrcErrorCount));
				ptr += sizeof(uplinkCrcErrorCount);
				[downlinkPacketCrcReportLabel setIntValue:uplinkCrcErrorCount];
				
				unsigned short packetCrc;
				memcpy(&packetCrc, ptr, sizeof(packetCrc));
				ptr+=sizeof(packetCrc);
				[downlinkPacketCrcLabel setIntValue:packetCrc];
				
				// Remove the handled frame
				memcpy(downlinkFrame, &downlinkFrame[SIZE_FULL_FRAME], downlinkFrameIndex-SIZE_FULL_FRAME);
				downlinkFrameIndex -= SIZE_FULL_FRAME;
			}
			else 
			{
				// TODO: increment some counter
				
				NSLog(@"Downlink bad CRC (below)");
				
				for(int i=0; i<SIZE_FULL_FRAME; i++)
				{
						printf("%.2X ", downlinkFrame[i]);
				}
				
				// Remove the bad frame
				memcpy(downlinkFrame, &downlinkFrame[SIZE_FULL_FRAME], downlinkFrameIndex-SIZE_FULL_FRAME);
				downlinkFrameIndex -= SIZE_FULL_FRAME;
			}

		}

		
#endif
	}
	[pool drain];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	
	// TODO: Add observer to only one struct, not three vars, easier to observe.
	[self addObserver:self forKeyPath:@"heading" options:NSKeyValueObservingOptionOld context:nil];
	
	calculatedPulsePitch = 1500;
	calculatedPulseHeading = 1500;
	
//	unsigned char testBuffer[] = {0xEF, 0xBE, 0x06, 0x09, 0x01, 0xb0};
//	uint16_t crc = 0xffff;
//	crc = crc16_update(crc, 0x06);
//	crc = crc16_update(crc, 0x09);
//	
//	NSLog(@"crc single call resulted in:  %hu", crc);
//	
//	crc = crc16_array_update(&testBuffer[2], 2);
//	
//	NSLog(@"crc arra call resulted in:  %hu", crc);
//	
//	if (crc16_verify(testBuffer, 6)) {
//		NSLog(@"it verifies");
//	}
//	else {
//		NSLog(@"it dont verify");
//	}

	
}

- (IBAction)startReadingSensorData:(id)sender
{
	serialReadFileDescriptor = [SerialReaderAppDelegate openPort];
	readerThread = [[NSThread alloc] initWithTarget:self selector:@selector(readerLoop) object:nil];
	[readerThread start];
	
	serialWriteFileDescriptor = [SerialReaderAppDelegate openDownlinkPort];
	downlinkThread = [[NSThread alloc] initWithTarget:self selector:@selector(downlinkReaderLoop) object:nil];
	[downlinkThread start];
	
}

- (IBAction)stopReadingSensorData:(id)sender
{
	[readerThread cancel];
	[readerThread release];
	readerThread = nil;
	[downlinkThread cancel];
	[downlinkThread release];
	downlinkThread = nil;
	close_port(serialReadFileDescriptor);
	close_port(serialWriteFileDescriptor);
}

- (void) observeValueForKeyPath:(NSString *)keyPath 
					   ofObject:(id)object 
						 change:(NSDictionary *)change 
						context:(void *)context
{
	// Update the OpenGL View
	[theSensorView setSensorHeading:heading];
	[theSensorView setSensorPitch:pitch];
	[theSensorView setSensorRoll:roll];
	[theSensorView setNeedsDisplay:YES];
	
#pragma mark Heading Pulse Calculation

	float headingDelta = heading - prevHeading;
	
	// Handle the case where we pass 0 degrees
	if (headingDelta > 180.0) headingDelta -= 360.0;
	else if(headingDelta < -180.0) headingDelta += 360.0;

	// Calculate the pan servo pulse width
	float deltaFloat = headingDelta * 1000.0/180.0;
	calculatedPulseHeading += deltaFloat;
	
	// Limit the servo pwm range
	if(calculatedPulseHeading > 2000) calculatedPulseHeading = 2000;
	else if(calculatedPulseHeading < 1000) calculatedPulseHeading = 1000;
	
	// Round the actual servo pulse width
	unsigned short servoPulseHeading = (short)calculatedPulseHeading;
	
	[headingPulseLabel setIntegerValue:servoPulseHeading];
	NSNumberFormatter *floatFieldFormatter = [[[NSNumberFormatter alloc]init]autorelease];
	[floatFieldFormatter setFormat:@"##0.0"];
	[headingLabel setFormatter:floatFieldFormatter];
	[headingLabel setFloatValue:heading];

	prevHeading = heading;

	//NSLog(@"Heading Servo Pulse: %f", calculatedPulseHeading);	
	
#pragma mark Pitch Pulse Calculation
	
	// TODO: calculate pitch
	unsigned short servoPulsePitch = 0x0001;
	
	[pitchPulseLabel setIntValue:servoPulsePitch];
	
#pragma mark Build and Send Uplink Packet

	ushort header = 0xBEEF;
	
	// TODO: Make "Prepare Packet" routine, takes payload, payload type, returns full prepared packet with crc.
	
	unsigned char buffer[PACKET_SIZE_HEADTRACKER];
	int offset = 0;
	
	memcpy(&buffer[offset], &header, sizeof(header));
	offset += sizeof(header);
	
	memcpy(&buffer[offset], &servoPulseHeading, sizeof(short));
	offset += sizeof(short);
	
	memcpy(&buffer[offset], &servoPulsePitch, sizeof(short));
	offset += sizeof(short);
	
	uint16_t crc = crc16_array_update(&buffer[2], PAYLOAD_SIZE_HEADTRACKER);
	
	// Append the crc to the end of the packet
	memcpy(&buffer[PACKET_SIZE_HEADTRACKER-2], &crc, sizeof(crc));
	
	NSString *bytesAsString = [[[NSString alloc] init] autorelease];
	
	for(int i=0; i< PACKET_SIZE_HEADTRACKER; i++)
	{
		bytesAsString = [bytesAsString stringByAppendingFormat:@"%.2X ",buffer[i]];
	}
	
	// Set GUI Uplink Frame Label
	[uplinkpacketFrameLabel setStringValue:bytesAsString];
	[uplinkPacketCrcLabel setIntValue:crc];
	
	crc16_verify(buffer, PACKET_SIZE_HEADTRACKER);
	//NSLog(@"Tx %@", bytesAsString);
	
	//unsigned char testbuffer[PACKET_SIZE_HEADTRACKER] = {0xEF, 0xBE, 0x25, 0x05, 0x01, 0x00, 0xF1, 0x0D};
	// Send the Packet
	write_uplink(serialWriteFileDescriptor, buffer, PACKET_SIZE_HEADTRACKER);
}



@end
