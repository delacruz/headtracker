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
#define PAYLOAD_SIZE_HEADTRACKER 5
#define PACKET_SIZE_HEADTRACKER HEADER_SIZE + PAYLOAD_SIZE_HEADTRACKER + CRC_SIZE

NSString * const KeyServoPulseMinPan = @"PanServoMinPulse";
NSString * const KeyServoPulseMaxPan = @"PanServoMaxPulse";
NSString * const KeyServoPulseMinTilt = @"TiltServoMinPulse";
NSString * const KeyServoPulseMaxTilt = @"TiltServoMaxPulse";

@implementation SerialReaderAppDelegate

@synthesize window;

+ (void)initialize
{
	NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
	[defaultValues setObject:[NSNumber numberWithInt:1000] forKey:KeyServoPulseMinPan];
	[defaultValues setObject:[NSNumber numberWithInt:2000] forKey:KeyServoPulseMaxPan];
	[defaultValues setObject:[NSNumber numberWithInt:1000] forKey:KeyServoPulseMinTilt];
	[defaultValues setObject:[NSNumber numberWithInt:2000] forKey:KeyServoPulseMaxTilt];
	[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
	NSLog(@"Default values registered.");
}

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
		[self willChangeValueForKey:@"pitch"];
		[self willChangeValueForKey:@"roll"];
		read_sensor_port(serialReadFileDescriptor, &heading, &pitch, &roll);
		[self didChangeValueForKey:@"heading"];
		[self didChangeValueForKey:@"pitch"];
		[self didChangeValueForKey:@"roll"];
	}
	[pool drain];
}

void trimToHeader(unsigned char* buffer, unsigned char* index)
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

#define PRINTRX 1

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
		
		trimToHeader(downlinkFrame, &downlinkFrameIndex);
		
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
				
				// Chop head
				memcpy(downlinkFrame, &downlinkFrame[2], downlinkFrameIndex-2);
				downlinkFrameIndex -= 2;
				trimToHeader(downlinkFrame, &downlinkFrameIndex);
			}
		}
		
#endif
	}
	[pool drain];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	
	// TODO: Add observer to only one struct, not three vars, easier to observe.
	[self addObserver:self forKeyPath:@"heading" options:NSKeyValueObservingOptionOld context:nil];
	[self addObserver:self forKeyPath:@"pitch" options:NSKeyValueObservingOptionOld context:nil];
	[self addObserver:self forKeyPath:@"roll" options:NSKeyValueObservingOptionOld context:nil];
	
	NSNumberFormatter *floatFieldFormatter = [[[NSNumberFormatter alloc]init]autorelease];
	[floatFieldFormatter setFormat:@"##0.0"];
	[headingLabel setFormatter:floatFieldFormatter];
	[pitchLabel setFormatter:floatFieldFormatter];
	[rollLabel setFormatter:floatFieldFormatter];
	
	calculatedPulsePitch = 1500;
	calculatedPulseHeading = 1500;
	
	[self willChangeValueForKey:@"servoPulseMinPan"];
	[self willChangeValueForKey:@"servoPulseMaxPan"];
	[self willChangeValueForKey:@"servoPulseMinTilt"];
	[self willChangeValueForKey:@"servoPulseMaxTilt"];
	
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	servoPulseMinPan = [defaults integerForKey:KeyServoPulseMinPan];
	servoPulseMaxPan = [defaults integerForKey:KeyServoPulseMaxPan];
	servoPulseMinTilt = [defaults integerForKey:KeyServoPulseMinTilt];
	servoPulseMaxTilt = [defaults integerForKey:KeyServoPulseMaxTilt];
	
	[self didChangeValueForKey:@"servoPulseMinPan"];
	[self didChangeValueForKey:@"servoPulseMaxPan"];
	[self didChangeValueForKey:@"servoPulseMinTilt"];
	[self didChangeValueForKey:@"servoPulseMaxTilt"];
	
	lockUplinkPacket = [[NSLock alloc] init]; // TODO: release this on dispose?
}

- (IBAction)startReadingSensorData:(id)sender
{
	serialReadFileDescriptor = [SerialReaderAppDelegate openPort];
	readerThread = [[NSThread alloc] initWithTarget:self 
										   selector:@selector(readerLoop) 
											 object:nil];
	[readerThread start];
	
	serialWriteFileDescriptor = [SerialReaderAppDelegate openDownlinkPort];
	downlinkThread = [[NSThread alloc] initWithTarget:self 
											 selector:@selector(downlinkReaderLoop) 
											   object:nil];
	[downlinkThread start];
	
	uplinkTimer = [NSTimer scheduledTimerWithTimeInterval: 0.05
												   target: self
												 selector: @selector(onUplinkTimer:)
												 userInfo: nil
												  repeats: YES];
	[[NSRunLoop currentRunLoop] addTimer:uplinkTimer forMode:NSEventTrackingRunLoopMode];
	
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
	
	[uplinkTimer invalidate];
	[uplinkTimer release];
	uplinkTimer = nil;
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
	float deltaFloat = headingDelta * ((float)(servoPulseMaxPan - servoPulseMinPan)) / 180.0;
	calculatedPulseHeading += deltaFloat;
	
	// Enforce range limit if reached
	if(calculatedPulseHeading > servoPulseMaxPan) calculatedPulseHeading = servoPulseMaxPan;
	else if(calculatedPulseHeading < servoPulseMinPan) calculatedPulseHeading = servoPulseMinPan;
	
	// Round the actual servo pulse width
	unsigned short servoPulseHeading = (unsigned short)calculatedPulseHeading;
	
	[headingPulseLabel setIntegerValue:servoPulseHeading];
	
//	[headingLabel setFloatValue:heading];

	prevHeading = heading;

	//NSLog(@"Heading Servo Pulse: %f", calculatedPulseHeading);	
	
#pragma mark Pitch Pulse Calculation
	
	// Delta
	float pitchDelta = pitch - prevPitch;
	
	// Calculate pulse width
	calculatedPulsePitch -= pitchDelta * ((float)(servoPulseMaxTilt-servoPulseMinTilt)) / 90.0;
	
	// Enforce range limit if reached
	if (calculatedPulsePitch > servoPulseMaxTilt) calculatedPulsePitch = servoPulseMaxTilt;
	else if (calculatedPulsePitch < servoPulseMinTilt) calculatedPulsePitch = servoPulseMinTilt;
	
	unsigned short servoPulsePitch = (unsigned short)calculatedPulsePitch;

	[pitchPulseLabel setIntValue:servoPulsePitch];
	
	prevPitch = pitch;
	
#pragma mark Build and Send Uplink Packet

	ushort header = 0xBEEF;
	
	// TODO: Make "Prepare Packet" routine, takes payload, payload type, returns full prepared packet with crc.
	
	unsigned char buffer[PACKET_SIZE_HEADTRACKER];
	int offset = 0;
	
	memcpy(&buffer[offset], &header, sizeof(header));
	offset += sizeof(header);
	
	buffer[offset] = 0x32;
	offset += 1;
	
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
	
	// Ready to be sent, copy to uplinkPacket memory
	@synchronized(lockUplinkPacket)
	{
		memcpy(uplinkPacket, buffer, PACKET_SIZE_HEADTRACKER);
	}
}

- (IBAction)sendBadByte:(id)sender
{
	unsigned char badness[] = {0xff};
	write_uplink(serialWriteFileDescriptor, badness, 1);
}

- (void)onUplinkTimer:(NSTimer*)timer
{
	@synchronized(lockUplinkPacket)
	{
		write_uplink(serialWriteFileDescriptor, uplinkPacket, PACKET_SIZE_HEADTRACKER);
	}
}

- (IBAction)servoCalibrationChanged:(id)sender
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	[defaults setInteger:servoPulseMinPan forKey:KeyServoPulseMinPan];
	[defaults setInteger:servoPulseMaxPan forKey:KeyServoPulseMaxPan];
	[defaults setInteger:servoPulseMinTilt forKey:KeyServoPulseMinTilt];
	[defaults setInteger:servoPulseMaxTilt forKey:KeyServoPulseMaxTilt];
	
	[self observeValueForKeyPath:@"servoPulseMinPan" ofObject:nil change:nil context:nil];
}

- (IBAction)centerServos:(id)sender
{

}

@end
