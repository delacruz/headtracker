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
*   @file   Args.h
*
*   @brief  Provides simple support for string based arguments from EEPROM
*
****************************************************************************/

#if !defined( ARGS_H )
#define ARGS_H                   ///< Include Guard

// ---- Include Files -------------------------------------------------------

#include <stdint.h>

// ---- Constants and Types -------------------------------------------------

// ---- Variable Externs ----------------------------------------------------

// ---- Function Prototypes -------------------------------------------------

uint8_t NumArgs( void );
uint8_t GetArg( uint8_t argIdx, char *argBuf, uint8_t bufLen );

#endif  // ARGS_H

