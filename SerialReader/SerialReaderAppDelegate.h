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
	double heading, pitch, roll;
	double prevHeading, prevPitch, prevRoll;
	IBOutlet NSTextField *headingLabel;
	IBOutlet NSTextField *pitchLabel;
	IBOutlet NSTextField *rollLabel;
	IBOutlet NSSlider *headingSlider;
	IBOutlet NSSlider *pitchSlider;
	IBOutlet NSSlider *rollSlider;
	int serialReadFileDescriptor;
	int serialWriteFileDescriptor;
//	short servoPulseHeading;
//	short servoPulsePitch;
	short servoPrevPulseHeading;
	short servoPrevPulstPitch;
	NSThread *readerThread;
	NSThread *writerThread;
	IBOutlet SensorView *theSensorView;
}

@property (assign) IBOutlet NSWindow *window;

- (IBAction)startReadingSensorData:(id)sender;
- (IBAction)stopReadingSensorData:(id)sender;

@end
