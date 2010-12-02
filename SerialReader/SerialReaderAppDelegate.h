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
	
	IBOutlet NSTextField *headingPulseLabel;
	IBOutlet NSTextField *pitchPulseLabel;
	IBOutlet NSTextField *uplinkpacketFrameLabel;
	IBOutlet NSTextField *uplinkPacketCrcLabel;
	IBOutlet NSTextField *downlinkPacketFrameLabel;
	IBOutlet NSTextField *downlinkPacketCrcReportLabel;
	IBOutlet NSTextField *downlinkPacketCrcLabel;
	IBOutlet NSTextField *downlinkPacketSignalStrengthLabel;
	IBOutlet NSTextField *downlinkPacketFrameNumberLabel;
	
	NSTimer *uplinkTimer;
	
	unsigned char uplinkPacket[8];
	NSLock *lockUplinkPacket;
	
	int servoPulseMinPan, servoPulseMaxPan;
	int servoPulseMinTilt, servoPulseMaxTilt;
	unsigned int missedDownlinkFrameCount;
	
	IBOutlet NSButton *calibrateEnableButton;
	IBOutlet NSBox *calibrationBoxPanServo;
	IBOutlet NSBox *calibrationBoxTiltServo;
	
	IBOutlet NSTextField *linkDiagnosticsMissedDlFrameCountLabel;
	
}

@property (assign) IBOutlet NSWindow *window;

- (IBAction)startReadingSensorData:(id)sender;
- (IBAction)stopReadingSensorData:(id)sender;
- (IBAction)sendBadByte:(id)sender;
- (IBAction)servoCalibrationChanged:(id)sender;
- (IBAction)centerServos:(id)sender;

@end
