//
//  SensorReaderAppDelegate.m
//  SensorReader
//
//  Created by Jason de la Cruz on 8/31/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import "SensorReaderAppDelegate.h"
#import "sensor_data.h"

extern int open_port(void);
extern void close_port(int);
extern sensor_data_struct read_serial(int);


@implementation SensorReaderAppDelegate

@synthesize window;

+ (int)openPort
{
	return open_port();
}

+ (void)closePort: (int)withFileDescriptor
{
	
}

- (void)readerLoop
{
	// Runs on separate thread, needs its own autorelease pool
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	// Go until thread is cancelled
	while (![[NSThread currentThread] isCancelled])
	{
		// Read available data
		sensor_data_struct data = read_serial(serialFileDescriptor);
		
		NSString *headingString = [NSString stringWithFormat:@"%4.1f", data.heading];
		NSString *pitchString = [NSString stringWithFormat:@"%4.1f", data.pitch];
		NSString *rollString = [NSString stringWithFormat:@"%4.1f", data.roll];
		
		NSLog(@"heading: %@, pitch: %@, roll:%@", headingString, pitchString, rollString);
		
		// Update GUI labels
		[heading setStringValue:headingString];
		[pitch setStringValue:pitchString];
		[roll setStringValue:rollString];
		
		// Update the OpenGL view
		[theSensorView setSensorHeading:data.heading];
		[theSensorView setSensorPitch:data.pitch];
		[theSensorView setSensorRoll:data.roll];
		[theSensorView setNeedsDisplay:YES];
	}
	[pool drain];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification 
{
	// Insert code here to initialize your application
}

- (IBAction)startReadingSensorData:(id)sender
{
	// Open the serial port
	serialFileDescriptor = [SensorReaderAppDelegate openPort];
	
	// Start reader thread
	readerThread = [[NSThread alloc] initWithTarget:self selector:@selector(readerLoop) object:nil];
	[readerThread start];
}

- (IBAction)stopReadingSensorData:(id)sender
{
	[readerThread cancel];
	[readerThread release];
	readerThread = nil;
	close_port(serialFileDescriptor);
}

@end
