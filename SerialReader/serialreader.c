/*
 *  serialreader.c
 *  SerialReader
 *
 *  Created by Jason de la Cruz on 8/31/10.
 *  Copyright 2010 Mourning Wood Software, LLC. All rights reserved.
 *
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include "serialreader.h"

void read_sensor_port(int fd, float *heading, float *pitch, float *roll)
{
	char buffer[255];
	
	read(fd, buffer, sizeof(buffer));
	sscanf(buffer, "$C%fP%fR%f", heading, pitch, roll);
}

int open_port(void)
{
	int fd; /* File descriptor for the port */ 
	
	fd = open("/dev/tty.SLAB_USBtoUART", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		/*
		 * Could not open the port.
		 */
		
		perror("could not open port - ");
	}
	else
	{
		fcntl(fd, F_SETFL, 0);
		printf("Port now open...");
		
		struct termios options;
		
		tcgetattr(fd, &options);
		
		cfsetspeed(&options, B19200);
		
		options.c_cflag |= (CLOCAL | CREAD);
		
		tcsetattr(fd, TCSANOW, &options);

	}
	return (fd);
}

int open_uplink_downlink_port(void)
{
	int fd; /* File descriptor for the port */ 
	
	fd = open("/dev/tty.KeySerial1", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		/*
		 * Could not open the port.
		 */
		
		perror("could not open port for writing");
	}
	else
	{
		fcntl(fd, F_SETFL, 0);
		printf("Port now open...");
		
		struct termios options;
		
		tcgetattr(fd, &options);
		
		cfsetspeed(&options, B38400);
		
		options.c_cflag |= (CLOCAL | CREAD);
		
		tcsetattr(fd, TCSANOW, &options);
		
	}
	return (fd);
	
}

void write_uplink(int fd, unsigned char *data, int length)
{
	if (fd!=0)
	{
		write(fd, data, length);
	}
}


char read_downlink(int fd, unsigned char *buffer)
{
	return read(fd, buffer, 128); // TODO: Define max buffer size in config or elsewhere
}

void close_port(int fd)
{
	if (fd==-1) return;
	close(fd);
	printf("Port %d now closed...", fd);
}