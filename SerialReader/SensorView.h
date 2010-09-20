//
//  SensorView.h
//  SerialReader
//
//  Created by Jason de la Cruz on 9/17/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SensorView : NSOpenGLView {
	IBOutlet NSMatrix *sliderMatrix;
	int displayList;
	float lightX, theta, radius;
	float sensorHeading, sensorPitch, sensorRoll;
}

@property(readwrite, assign) float sensorHeading, sensorPitch, sensorRoll;




@end

