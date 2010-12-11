/*
 *  crc.c
 *  PanTilt
 *
 *  Created by Jason de la Cruz on 11/27/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#include "crc16.h"
#include "header.h"

char crc16_verify(const void* array, uint8_t length)
{
	uint16_t crc = 0xffff;
	
	const uint8_t *ptr = (const uint8_t *)array;
	
	// Skip header
	ptr+=sizeof(header_t);
	
	length-=(sizeof(header_t)+sizeof(crc_t));
	
	crc = crc16_array_update(ptr, length);
	
	uint16_t packet_crc = 0;
	memcpy(&packet_crc, ptr+length, 2);  // length=length of payload

	return crc == packet_crc;
}

uint16_t crc16_array_update(const void* array, uint8_t length)
{
	const uint8_t *ptr = (const uint8_t*)array;
	uint16_t crc = 0xffff;
	while (length>0)
	{
		crc = _crc16_update(crc, *ptr++);
		length--;
	}
	
	return crc;
}