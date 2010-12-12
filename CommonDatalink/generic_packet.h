/*
 *  generic_packet.h
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 12/8/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#define CREATE_PACKETIZATION_CODE(DATATYPE, PACKET_KIND)								\
typedef struct																			\
{																						\
	header_t header;																	\
    DATATYPE message;																	\
	crc_t	 crc;																		\
																						\
}PACKED DATATYPE##_pkt;																	\
																						\
static DATATYPE##_pkt create_##DATATYPE##_pkt(DATATYPE message);						\
static DATATYPE##_pkt create_##DATATYPE##_pkt(DATATYPE message)							\
{																						\
	DATATYPE##_pkt s;																	\
	s.header.delimiter = DELIMITER;														\
	s.header.packet_kind = PACKET_KIND;													\
	s.message = message;																\
	s.crc = crc16_array_update(&s.message, sizeof(DATATYPE));							\
	return s;																			\
}																						\
																						\
static uint8_t verify_crc_##DATATYPE##_pkt(DATATYPE##_pkt pkt);							\
static uint8_t verify_crc_##DATATYPE##_pkt(DATATYPE##_pkt pkt)							\
{																						\
	return pkt.crc == crc16_array_update(&pkt.message, sizeof(pkt.message));			\
}																						\
														\

