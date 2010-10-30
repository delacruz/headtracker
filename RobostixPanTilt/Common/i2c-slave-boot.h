/****************************************************************************
*
*   Copyright (c) 2006 Dave Hylands     <dhylands@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation.
*
*   Alternatively, this software may be distributed under the terms of BSD
*   license.
*
*   See README and COPYING for more details.
*
****************************************************************************/
/**
*
*   @file   i2c-slave-boot.h
*
*   @brief  This file contains definitions which allow the slave code from
*           the bootloader to be used, rather than linking in i2c-slave.c
*
****************************************************************************/

#if !defined( I2C_SLAVE_BOOT_H )
#define I2C_SLAVE_BOOT_H         /**< Include Guard                        */

/* ---- Include Files ---------------------------------------------------- */

#include <inttypes.h>

#if !defined( I2C_SLAVE_H )
#   include "i2c-slave.h"
#endif

/* ---- Constants and Types ---------------------------------------------- */

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

void    AppBoot( void );

uint8_t I2C_SlaveBootHandler( void );
uint8_t I2C_SlaveBootInit( I2C_ProcessCommand processCmd );
int     I2C_SlaveBootProcessCommand( I2C_Data_t *packet );

#endif  // I2C_SLAVE_BOOT_H

