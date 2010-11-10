/*
 *  serialreader.h
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 11/7/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */

int open_port(void);
int open_uplink_downlink_port(void);
void close_port(int);
void read_sensor_port(int fd, float *heading, float *pitch, float *roll);
void write_uplink(int, unsigned char *, int length);
char read_downlink(int fd, unsigned char* buffer);