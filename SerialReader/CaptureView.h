//
//  CaptureView.h
//  SerialReader
//
//  Created by Jason de la Cruz on 1/19/11.
//  Copyright 2011 Mourning Wood Software, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
#import <QuartzCore/QuartzCore.h>

@interface CaptureView : NSView {
	QTCaptureLayer *captureLyer;
	
}

@end
