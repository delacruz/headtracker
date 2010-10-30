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
		[self willChangeValueForKey:@"heading"];
		[self willChangeValueForKey:@"pitch"];
		[self willChangeValueForKey:@"roll"];
		heading = data.heading;
		pitch = data.pitch;
		roll = data.roll;
		[self didChangeValueForKey:@"heading"];
		[self didChangeValueForKey:@"pitch"];
		[self didChangeValueForKey:@"roll"];
		
		NSLog(@"\nheading: %f, pitch: %f, roll:%f", heading, pitch, roll);
		
	}
	[pool drain];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	// Insert code here to initialize your application 
	
	[self addObserver:self forKeyPath:@"heading" options:NSKeyValueObservingOptionOld context:nil];
	[self addObserver:self forKeyPath:@"pitch" options:NSKeyValueObservingOptionOld context:nil];
	[self addObserver:self forKeyPath:@"roll" options:NSKeyValueObservingOptionOld context:nil];
	
	if (serialFileDescriptor!=-1) {
		[SerialReaderAppDelegate closePort:serialFileDescriptor];
	}
	
}

- (IBAction)startReadingSensorData:(id)sender
{
	serialFileDescriptor = [SerialReaderAppDelegate openPort];
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

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	[theSensorView setSensorHeading:heading];
	[theSensorView setSensorPitch:pitch];
	[theSensorView setSensorRoll:roll];
	[theSensorView setNeedsDisplay:YES];
	
	
}

@end
