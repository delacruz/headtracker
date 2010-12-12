//
//  UplinkWrapper.m
//  SerialReader
//
//  Created by Jason de la Cruz on 12/12/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import "UplinkWrapper.h"


@implementation UplinkWrapper

@synthesize headtrackerCmdPkt, modemCmdPkt;

-(unsigned short)panServoPulse
{
	return headtrackerCmdPkt.message.pan_servo_pulse;
}

-(unsigned short)tiltServoPulse
{
	return headtrackerCmdPkt.message.tilt_servo_pulse;
}

-(unsigned char)txPowerLevel
{
	return modemCmdPkt.message.tx_power_level;
}

// TODO: Exists in downlink too; make reusable
-(NSString*)headtrackerCmdPktAsString
{
	NSString *bytesAsString = [[[NSString alloc] init] autorelease];
	uint8_t *ptr = (uint8_t*)&headtrackerCmdPkt;
	int length = sizeof(headtrackerCmdPkt);
	while(length)
	{
		bytesAsString = [bytesAsString stringByAppendingFormat:@"%.2X ",*ptr++];
		length--;
	}
	
	return bytesAsString;
}

@end
