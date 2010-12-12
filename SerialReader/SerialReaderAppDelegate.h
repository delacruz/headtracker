//
//  SerialReaderAppDelegate.h
//  SerialReader
//
//  Created by Jason de la Cruz on 8/31/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SensorView.h"
#import "DownlinkWrapper.h"
#import "UplinkWrapper.h"

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
	
	DownlinkWrapper *downlinkWrapper;
	UplinkWrapper *uplinkWrapper;
	
	IBOutlet NSTextField *headingPulseLabel;
	IBOutlet NSTextField *pitchPulseLabel;
	IBOutlet NSTextField *uplinkpacketFrameLabel;
	IBOutlet NSTextField *uplinkPacketCrcLabel;
	IBOutlet NSTextField *downlinkPacketFrameLabel;
	IBOutlet NSTextField *downlinkPacketCrcReportLabel;
	IBOutlet NSTextField *downlinkPacketCrcLabel;
	IBOutlet NSTextField *downlinkPacketSignalStrengthLabel;
	IBOutlet NSTextField *downlinkPacketFrameNumberLabel;
	IBOutlet NSTextField *downlinkPacketTxPowerLevelLabel;
	
	IBOutlet NSPopUpButton *avTxPowerButton;
	
	NSTimer *uplinkTimer;
	
	unsigned char uplinkPacket[8];
	NSLock *lockUplinkPacket;
	
	int servoPulseMinPan, servoPulseMaxPan;
	int servoPulseMinTilt, servoPulseMaxTilt;
	unsigned int missedDownlinkFrameCount;
	
	IBOutlet NSButton *calibrateEnableButton;
	IBOutlet NSBox *calibrationBoxPanServo;
	IBOutlet NSBox *calibrationBoxTiltServo;	
}

@property (assign) IBOutlet NSWindow *window;
@property (retain) DownlinkWrapper *downlinkWrapper;
@property (retain) UplinkWrapper *uplinkWrapper;

- (IBAction)startReadingSensorData:(id)sender;
- (IBAction)stopReadingSensorData:(id)sender;
- (IBAction)sendBadByte:(id)sender;
- (IBAction)servoCalibrationChanged:(id)sender;
- (IBAction)centerServos:(id)sender;
- (IBAction)changeAvTxPowerLevel:(id)sender;

@end
