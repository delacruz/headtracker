//
//  SerialReaderAppDelegate.m
//  SerialReader
//
//  Created by Jason de la Cruz on 8/31/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import "SerialReaderAppDelegate.h"
#import "sensor_data.h"

extern int open_port(void);
extern void close_port(int);
extern sensor_data_struct read_serial(int);


@implementation SerialReaderAppDelegate

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
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	while (![[NSThread currentThread] isCancelled])
	{
		sensor_data_struct data = read_serial(serialFileDescriptor);
		NSLog(@"\nheading: %f, pitch: %f, roll:%f", data.heading, data.pitch, data.roll);
		NSString *headingString = [NSString stringWithFormat:@"%f", data.heading];
		NSString *pitchString = [NSString stringWithFormat:@"%f", data.pitch];
		NSString *rollString = [NSString stringWithFormat:@"%f", data.roll];
		[heading setStringValue:headingString];
		[pitch setStringValue:pitchString];
		[roll setStringValue:rollString];
		[theSensorView setSensorHeading:data.heading];
		[theSensorView setSensorPitch:data.pitch];
		[theSensorView setSensorRoll:data.roll];
		[theSensorView setNeedsDisplay:YES];
	}
	[pool drain];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 

	
	if (serialFileDescriptor!=-1) {
		[SerialReaderAppDelegate closePort:serialFileDescriptor];
	}
	
}

- (IBAction)startReadingSensorData:(id)sender
{
	serialFileDescriptor = [SerialReaderAppDelegate openPort];
	readerThread = [[NSThread alloc] initWithTarget:self selector:@selector(readerLoop) object:nil];
	[readerThread start];
	//[NSThread detachNewThreadSelector:@selector(readerLoop) toTarget:self withObject:nil];
	
}

- (IBAction)stopReadingSensorData:(id)sender
{
	[readerThread cancel];
	[readerThread release];
	readerThread = nil;
	close_port(serialFileDescriptor);
}

@end
