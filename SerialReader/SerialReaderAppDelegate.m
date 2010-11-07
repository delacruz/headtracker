//
//  SerialReaderAppDelegate.m
//  SerialReader
//
//  Created by Jason de la Cruz on 8/31/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import "SerialReaderAppDelegate.h"
#import "sensor_data.h"
#import "crc16.h"

// TODO: too many externs here, create and include .h file instead
extern int open_port(void);
extern int open_uplink_downlink_port(void);
extern void close_port(int);
extern sensor_data_struct read_sensor_port(int);
extern void write_uplink(int, char *, int length);
extern char read_downlink(int fd, unsigned char* buffer);

#define PACKET_SIZE_HEADTRACKER 8

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
		sensor_data_struct data = read_sensor_port(serialReadFileDescriptor);
		[self willChangeValueForKey:@"heading"];
		//[self willChangeValueForKey:@"pitch"];
		//[self willChangeValueForKey:@"roll"];
		heading = data.heading;
		pitch = data.pitch;
		roll = data.roll;
		[self didChangeValueForKey:@"heading"];
		//[self didChangeValueForKey:@"pitch"];
		//[self didChangeValueForKey:@"roll"];
		
	}
	[pool drain];
}

- (void)downlinkReaderLoop
{
	const int SIZE_FULL_FRAME = 8;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	unsigned char downlinkFrame[255];
	unsigned char buffer[255];
	unsigned char* framePtr;
	unsigned char frameIndex;
	unsigned char numBytesRead;
	
	framePtr = &downlinkFrame[0];
	
	while (![[NSThread currentThread] isCancelled] && serialWriteFileDescriptor != -1)
	{
		// Get new bytes
		numBytesRead = read_downlink(serialWriteFileDescriptor, buffer);
		
		// Append to our downlink frame
		memcpy(framePtr, buffer, numBytesRead);
		frameIndex += numBytesRead;
		framePtr = &downlinkFrame[frameIndex];

		if (frameIndex >= 1) 
		{
			// Verify frame starts with a header, otherwise reset
			unsigned short frameHead;
			memcpy(&frameHead, downlinkFrame, sizeof(short));
			if (frameHead != 0xBEEF)
			{
				printf("%x %x not header, resetting...\n", downlinkFrame[0], downlinkFrame[1]);
				frameIndex = 0;
				framePtr = &downlinkFrame[0];
				continue;
			}
		}

		// Full frame? Check CRC and handle accordingly
		if (frameIndex >= SIZE_FULL_FRAME-1) 
		{
			// TODO: implement, but for now just show what we got:
			int i;
			printf("\n Downlink Frame Contents: ");
			for(i=0; i<SIZE_FULL_FRAME; i++)
			{
				printf("%x", downlinkFrame[i]);
			}
			printf("\n");
			// Reset frame index pointer
			frameIndex = 0;
			framePtr = &downlinkFrame[0];
		}
		else 
		{
			NSLog(@"Incomplete - Total bytes so far: %d", frameIndex);
		}

	
	}
	[pool drain];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	
	// TODO: Add observer to only one struct, not three vars, easier to observe.
	[self addObserver:self forKeyPath:@"heading" options:NSKeyValueObservingOptionOld context:nil];
	[self addObserver:self forKeyPath:@"pitch" options:NSKeyValueObservingOptionOld context:nil];
	[self addObserver:self forKeyPath:@"roll" options:NSKeyValueObservingOptionOld context:nil];
	
	if (serialReadFileDescriptor!=-1) {
		[SerialReaderAppDelegate closePort:serialReadFileDescriptor];
	}
	calculatedPulsePitch = 1500;
	calculatedPulseHeading = 1500;
//	servoPrevPulsePitch = 1500;
//	servoPrevPulseHeading = 1500;
	
	//[downlinkPanel textStorage]
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
	
	// Round the actual servo pulse width
	short servoPulseHeading = (short)calculatedPulseHeading;
	
	// Limit the servo pwm range
	if(calculatedPulseHeading>2000) calculatedPulseHeading = 2000;
	else if(calculatedPulseHeading<1000) calculatedPulseHeading = 1000;

	prevHeading = heading;

	NSLog(@"Heading Servo Pulse: %f", calculatedPulseHeading);	
	
#pragma mark Pitch Pulse Calculation
	
	// TODO: calculate pitch
	short servoPulsePitch = 0xC0DE;
	
#pragma mark Build and Send Uplink Packet
	
	// TODO: Prepare Packet: 0xBEEF-[2-byte Heading]-[2-byte Pitch]-[CRC]
	ushort header = 0xBEEF;
	
	unsigned char buffer[PACKET_SIZE_HEADTRACKER];
	int offset = 0;
	
	// 
	memcpy(&buffer[offset], &header, sizeof(header));
	offset += sizeof(header);
	
	memcpy(&buffer[offset], &servoPulseHeading, sizeof(short));
	offset += sizeof(short);
	
	memcpy(&buffer[offset], &servoPulsePitch, sizeof(short));
	offset += sizeof(short);
	
	// TODO: Create crc_append function in crc.c and remove the crap below
	uint16_t crc = 0xffff;
	for(int i=1; i<(PACKET_SIZE_HEADTRACKER-1); i++) // i=1 because we don't care to crc the header; -1 because the last slot is for the crc
	{
		crc = crc16_update(crc, buffer[i]);
	}
	// Append the crc to the end of the packet
	memcpy(&buffer[PACKET_SIZE_HEADTRACKER-2], &crc, sizeof(crc));
	
	NSLog(@"the buffer is ready:");
	for(int i=0; i< PACKET_SIZE_HEADTRACKER; i++)
	{
		printf("%.2X ", buffer[i]);
	}
	
	//NSData *myData = [NSData dataWithBytes:buffer length:offset];

	//NSLog(@"offset: %d short size: %d byte: %@ \n",offset,1,myData);

	//write_serial(serialWriteFileDescriptor, <#char *data#>, <#int length#>)
}



@end
