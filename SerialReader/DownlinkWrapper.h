//
//  DownlinkWrapper.h
//  SerialReader
//
//  Created by Jason de la Cruz on 12/10/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "downlink.h"


@interface DownlinkWrapper : NSObject 
{
	downlink_t downlinkData;
}

@property(readonly) unsigned short badUplinkCrcCount;
@property(readonly) unsigned char signalStrength;
@property(readonly) unsigned char txPowerLevel;
@property(readonly) unsigned char counter;

@end
