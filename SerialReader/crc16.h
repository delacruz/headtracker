/*
 *  crc16.h
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 11/7/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#import <inttypes.h>

uint16_t crc16_update(uint16_t crc, uint8_t array);

uint16_t crc16_array_update(uint8_t array, uint8_t length);

char crc16_verify(void* array, uint8_t length);

