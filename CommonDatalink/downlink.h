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
	uint16_t	bad_uplink_crc_cnt;
	uint8_t		signal_strength;
	uint8_t		tx_power_level;
	uint8_t		counter;
}PACKED downlink_t;

CREATE_PACKETIZATION_CODE(downlink_t, KIND_DOWNLINK)

//typedef struct																	
//{																				
//	header_t header;															
//    downlink_t message;
//	crc_t	 crc;																
//	
//}PACKED downlink_t_pkt;															
//
//static downlink_t_pkt create_downlink_t_pkt(downlink_t message);						
//static downlink_t_pkt create_downlink_t_pkt(downlink_t message)						
//{																				
//	downlink_t_pkt s;															
//	s.header.delimiter = DELIMITER;												
//	s.header.packet_kind = KIND_DOWNLINK;											
//	s.message = message;														
//	s.crc = crc16_array_update(&s.message, sizeof(downlink_t));					
//	return s;																	
//}																				
//
//static uint8_t verify_crc_downlink_t_pkt(downlink_t_pkt pkt);						
//static uint8_t verify_crc_downlink_t_pkt(downlink_t_pkt pkt)							
//{																				
//	return pkt.crc == crc16_array_update(&pkt.message, sizeof(pkt.message));	
//}																				

#endif