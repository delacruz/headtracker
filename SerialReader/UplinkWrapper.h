//
//  UplinkWrapper.h
//  SerialReader
//
//  Created by Jason de la Cruz on 12/12/10.
//  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "uplink.h"

@interface UplinkWrapper : NSObject {

	headtracker_cmd_t_pkt headtrackerCmdPkt;
	modem_cmd_t_pkt modemCmdPkt;
}

@property (readwrite, assign) headtracker_cmd_t_pkt headtrackerCmdPkt;
@property (readwrite, assign) modem_cmd_t_pkt modemCmdPkt;

-(unsigned short)panServoPulse;
-(unsigned short)tiltServoPulse;
-(unsigned char)txPowerLevel;
-(NSString*)headtrackerCmdPktAsString;

@end
