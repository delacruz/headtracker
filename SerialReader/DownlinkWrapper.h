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
	downlink_t_pkt downlinkData;
}

@property (readwrite, assign) downlink_t_pkt downlinkData;

-(unsigned short)badUplinkCrcCount;
-(unsigned char)signalStrength;
-(unsigned char)txPowerLevel;
-(bool)isVideoRelayOn;
-(bool)isFailsafeModeOn;
-(bool)isFailsafeComOk;
-(unsigned char)counter;
-(NSString*)downlinkPktAsString;

@end
