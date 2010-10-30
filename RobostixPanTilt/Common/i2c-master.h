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
*   @file   i2c-master.h
*
*   @brief  Contains definitions for the master portion of the i2c support.
*
****************************************************************************/

#if !defined( I2C_MASTER_H )
#define I2C_MASTER_H        /**< Include Guard                             */

/* ---- Include Files ---------------------------------------------------- */

#include <string.h>
#include <avr/io.h>

#include "i2c.h"

/**
 *  @addtogroup I2C
 *  @{
 */

/* ---- Constants and Types ---------------------------------------------- */

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

#if defined( __cplusplus )
extern "C"
{
#endif

void I2C_Init( I2C_Addr_t addr );
uint8_t I2C_MasterCall( I2C_Addr_t slaveAddr, const I2C_Data_t *writeData, I2C_Data_t *readData );
uint8_t I2C_MasterRead( I2C_Addr_t slaveAddr, I2C_Data_t *readData );
uint8_t I2C_MasterWrite( I2C_Addr_t slaveAddr, const I2C_Data_t *writeData );

#if defined( __cplusplus )
}
#endif

/** @} */

#endif /* I2C_MASTER_H */

