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
#include "sensor_data.h"


sensor_data_struct read_serial(int fd)
{
	sensor_data_struct sensor_data;
	char buffer[255];

	float heading, pitch, roll;
	
	read(fd, buffer, sizeof(buffer));
	sscanf(buffer, "$C%fP%fR%f", &heading, &pitch, &roll);
//	
//		printf("\nread: %d: %s",readLen, buffer);
//		printf("\ntranslates to heading: %f, pitch: %f, roll:%f", heading, pitch, roll);
	sensor_data.heading = heading;
	sensor_data.pitch = pitch;
	sensor_data.roll = roll;

	return sensor_data;
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



void close_port(int fd)
{
	close(fd);
	printf("Port now closed...");
}