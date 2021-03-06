/*
 *  downlink.h
 *  PanTilt
 *
 *  Created by Jason de la Cruz on 12/5/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#ifndef DOWNLINK_H
#define DOWNLINK_H

#include "header.h"
#include "crc16.h"
#include "generic_packet.h"

typedef struct
{
	uint16_t bad_uplink_crc_cnt;
	uint8_t	 signal_strength;
	unsigned tx_power_level		:3;
	unsigned is_video_relay_on	:1;
	unsigned is_failsafe_mode	:1;
	unsigned is_failsafe_com_ok	:1;
	unsigned ___padding___		:2;
	uint8_t	 counter;
}PACKED downlink_t;

CREATE_PACKETIZATION_CODE(downlink_t, KIND_DOWNLINK)
																			
#undef CREATE_PACKETIZATION_CODE

#endif