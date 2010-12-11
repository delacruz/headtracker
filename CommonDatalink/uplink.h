/*
 *  uplink.h
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 12/5/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#ifndef UPLINK_H
#define UPLINK_H

#include "header.h"
#include "generic_packet.h"

// Headtracker Command
typedef struct
{
	uint16_t	pan_servo_pulse;
	uint16_t	tilt_servo_pulse;
	
}PACKED headtracker_cmd_t;

CREATE_PACKETIZATION_CODE(headtracker_cmd_t, KIND_HEADTRACKER_CMD)

// Modem Configuration Command
typedef struct
{
	uint8_t tx_power_level;
}PACKED modem_cmd_t;

//typedef struct
//{
//	header_t	header;
//	modem_cmd_t	cfg;
//	crc_t		crc;
//}PACKED modem_cmd_pkt_t;

CREATE_PACKETIZATION_CODE(modem_cmd_t, KIND_MODEM_CMD)

#endif