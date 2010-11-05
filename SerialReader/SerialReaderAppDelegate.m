//
//  SerialReaderAppDelegate.m
//  SerialReader
//
//  Created by Jason de la Cruz on 8/31/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import "SerialReaderAppDelegate.h"
#import "sensor_data.h"
#import "Crc8.h"

extern int open_port(void);
extern int open_write_port(void);
extern void close_port(int);
extern sensor_data_struct read_serial(int);
extern void write_serial(int, char *, int length);


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
		sensor_data_struct data = read_serial(serialReadFileDescriptor);
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

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	
	// TODO: Add observer to only one struct, not three vars, easier to observe.
	[self addObserver:self forKeyPath:@"heading" options:NSKeyValueObservingOptionOld context:nil];
	//[self addObserver:self forKeyPath:@"pitch" options:NSKeyValueObservingOptionOld context:nil];
	//[self addObserver:self forKeyPath:@"roll" options:NSKeyValueObservingOptionOld context:nil];
	
	if (serialReadFileDescriptor!=-1) {
		[SerialReaderAppDelegate closePort:serialReadFileDescriptor];
	}
	//servoPulsePitch = 1500;
	//servoPulseHeading = 1500;
	servoPrevPulstPitch = 1500;
	servoPrevPulseHeading = 1500;
}

- (IBAction)startReadingSensorData:(id)sender
{
	serialReadFileDescriptor = [SerialReaderAppDelegate openPort];
	readerThread = [[NSThread alloc] initWithTarget:self selector:@selector(readerLoop) object:nil];
	[readerThread start];
}

- (IBAction)stopReadingSensorData:(id)sender
{
	[readerThread cancel];
	[readerThread release];
	readerThread = nil;
	close_port(serialReadFileDescriptor);
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	// Update the OpenGL View
	[theSensorView setSensorHeading:heading];
	[theSensorView setSensorPitch:pitch];
	[theSensorView setSensorRoll:roll];
	[theSensorView setNeedsDisplay:YES];
	
	//NSLog(@"\nheading: %f, pitch: %f, roll:%f", heading, pitch, roll);
	
	

	double headingDelta = heading - prevHeading;
	
	// Handle the case where we pass 0 degrees
	if (headingDelta >= 180.0) headingDelta -= 360.0;
	else if(headingDelta <= -180.0) headingDelta += 360.0;

	// Calculate the pan servo pulse width
	short servoPulseHeading = 1500;
	servoPulseHeading += (short)(headingDelta * 1000.0/180.0);
	
	// Limit the servo pwm range
	if(servoPulseHeading>2000) servoPulseHeading = 2000;
	else if(servoPulseHeading<1000) servoPulseHeading = 1000;
	
	// If the change was significant enough to affect 1pw int, update
	if(servoPrevPulseHeading!=servoPulseHeading)
	{
		prevHeading = heading;
		servoPrevPulseHeading = servoPulseHeading;
		NSLog(@"Heading Servo Pulse: %d", servoPulseHeading);
	}
		

	
	// Send the data
	// TODO: Prepare Packet: 0xBEEF-[2-byte Heading]-[2-byte Pitch]-[CRC]
	ushort header = 0xBEEF;
	char buffer[255];
	int offset = 0;
	
	memcpy(&buffer, &header, sizeof(header));
	offset += sizeof(header);
	
	
	
	//NSData *myData = [NSData dataWithBytes:buffer length:offset];

	//NSLog(@"offset: %d short size: %d byte: %@ \n",offset,1,myData);

	//write_serial(serialWriteFileDescriptor, <#char *data#>, <#int length#>)
}

@end
