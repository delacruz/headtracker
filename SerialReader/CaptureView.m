//
//  CaptureView.m
//  SerialReader
//
//  Created by Jason de la Cruz on 1/19/11.
//  Copyright 2011 Mourning Wood Software, LLC. All rights reserved.
//

#import "CaptureView.h"


@implementation CaptureView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    // Drawing code here.
}

- (void)awakeFromNib
{
	[self setLayer:[CALayer layer]];
	[self setWantsLayer:YES];
	
	// Create the QTCapture Layer
	QTCaptureSession *session;
	NSError *error;
	
	session = [[QTCaptureSession alloc]init];
	
	// Get buit in isite for now
	QTCaptureDevice *device = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
	
	[device open:&error];
	
	QTCaptureDeviceInput *input = [[QTCaptureDeviceInput alloc] initWithDevice:device];
	
	[session addInput:input error:&error];
	
	captureLyer = [[QTCaptureLayer alloc] initWithSession:session];
	[captureLyer setFrame:CGRectMake(0.0, 
								   0.0, 
								   [self frame].size.width, 
								   [self frame].size.height)];
	
	
	[[self layer] addSublayer:captureLyer];
	
	[session startRunning];
	
	
}

@end
