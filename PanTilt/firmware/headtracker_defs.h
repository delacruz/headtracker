/*
 *  headtracker_defs.h
 *  PanTilt
 *
 *  Created by Jason de la Cruz on 11/27/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */
#if !defined( HEADTRACKER_DEFS_H )
#define HEADTRACKER_DEFS_H

#define HEADER_SIZE									2
#define CRC_SIZE									2
#define FRAME_TYPE_ID_SIZE							1

#define UPLINK_REPORT_PACKET_SIZE					9

#define FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND		0x32
#define FRAME_TYPE_UPLINK_HEADTRACKER_COMMAND_SIZE	9

#define FRAME_TYPE_UPLINK_MODEM_CONFIG				0x33
#define FRAME_TYPE_UPLINK_MODEM_CONFIG_SIZE			6

#endif // HEADTRACKER_DEFS_H