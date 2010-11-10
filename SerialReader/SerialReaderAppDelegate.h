//
//  SerialReaderAppDelegate.h
//  SerialReader
//
//  Created by Jason de la Cruz on 8/31/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SensorView.h"

@interface SerialReaderAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
	float heading, pitch, roll;
	//sensor_data_struct sensorData;
	float prevHeading, prevPitch, prevRoll;
	IBOutlet NSTextField *headingLabel;
	IBOutlet NSTextField *pitchLabel;
	IBOutlet NSTextField *rollLabel;
	IBOutlet NSSlider *headingSlider;
	IBOutlet NSSlider *pitchSlider;
	IBOutlet NSSlider *rollSlider;
	int serialReadFileDescriptor;
	int serialWriteFileDescriptor;
	float calculatedPulseHeading;
	float calculatedPulsePitch;
	NSThread *readerThread;
	NSThread *downlinkThread;
	IBOutlet SensorView *theSensorView;
}

@property (assign) IBOutlet NSWindow *window;

- (IBAction)startReadingSensorData:(id)sender;
- (IBAction)stopReadingSensorData:(id)sender;

@end
