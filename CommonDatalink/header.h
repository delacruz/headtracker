/*
 *  header.h
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 12/5/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#ifndef HEADER_H
#define HEADER_H

#include <inttypes.h>
#include "packet_kinds.h"

#define PACKED __attribute__((__packed__))
#define DELIMITER 0xBEEF

typedef uint16_t	delim_t;
typedef uint16_t	crc_t;

typedef struct
{
	delim_t		delimiter;
	kind_t		packet_kind;
}PACKED header_t;

#endif