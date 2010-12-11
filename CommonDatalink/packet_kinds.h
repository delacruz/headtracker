/*
 *  packet_kinds.h
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 12/5/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

#ifndef PACKET_KINDS_H
#define PACKET_KINDS_H

typedef uint8_t		kind_t;

// Uplink Packet Kinds
#define KIND_HEADTRACKER_CMD	0x32
#define KIND_MODEM_CMD			0x33

// Downlink Packet Kinds
#define KIND_DOWNLINK		0x80

#endif