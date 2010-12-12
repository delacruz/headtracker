//
//  DownlinkWrapper.m
//  SerialReader
//
//  Created by Jason de la Cruz on 12/10/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import "DownlinkWrapper.h"


@implementation DownlinkWrapper

@synthesize downlinkData;

-(unsigned short)badUplinkCrcCount
{
	return downlinkData.message.bad_uplink_crc_cnt;
}

- (unsigned char) signalStrength
{
	return downlinkData.message.signal_strength;
}

-(unsigned char)txPowerLevel
{
	return downlinkData.message.tx_power_level;
}

-(unsigned char)counter
{
	return downlinkData.message.counter;
}

-(unsigned short)crc
{
	return downlinkData.crc;
}

@end