/*
 *  crc.h
 *  PanTilt
 *
 *  Created by Jason de la Cruz on 11/27/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#ifndef CRC16_H
#define CRC16_H

#include <util/crc16.h>

uint16_t crc16_array_update(const void* array, uint8_t length);

char crc16_verify(const void* array, uint8_t length);


#endif