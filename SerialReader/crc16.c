/*
 *  crc16.c
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 11/7/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#include "crc16.h"
#include <string.h>

uint16_t crc16_update(uint16_t crc, uint8_t a)
{
	int i;
	
	crc ^= a;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = (crc >> 1);
	}
	
	return crc;
}

uint16_t crc16_array_update(uint8_t array, uint8_t length)
{
	uint16_t crc = 0xffff;
	while (length>0)
	{
		crc = crc16_update(crc, array++);
		length--;
	}
	
	return crc;
}

char crc16_verify(void* array, uint8_t length)
{
	uint16_t crc = 0xffff;
	
	uint8_t *ptr = (uint8_t *)array;
	
	// Skip header
	ptr+=2;  // TODO: define HEADER_SIZE somewhere, and include
	
	length-=4; // TODO: define CRC_SIZE somewhere, and include
	
	crc = crc16_array_update(*ptr, length);
	
	uint16_t packet_crc;
	memcpy(&packet_crc, ptr+length, 2);  // length=length of payload
	
	return crc == packet_crc;
}