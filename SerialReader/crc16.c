/*
 *  crc16.c
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 11/7/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#include "crc16.h"

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