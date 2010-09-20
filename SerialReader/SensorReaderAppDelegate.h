//
//  SensorReaderAppDelegate.h
//  SensorReader
//
//  Created by Jason de la Cruz on 8/31/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SensorView.h"

@interface SensorReaderAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
	IBOutlet NSTextField *heading;
	IBOutlet NSTextField *pitch;
	IBOutlet NSTextField *roll;
	int serialFileDescriptor;
	NSThread *readerThread;
	IBOutlet SensorView *theSensorView;
}

@property (assign) IBOutlet NSWindow *window;

- (IBAction)startReadingSensorData:(id)sender;
- (IBAction)stopReadingSensorData:(id)sender;

@end
