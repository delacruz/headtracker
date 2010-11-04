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
*   @file    Args.c
*
*   @brief   Provides simple support for string based arguments from EEPROM
*
*****************************************************************************/

// ---- Include Files -------------------------------------------------------

#include <avr/eeprom.h>

#include "Args.h"
#include "Config.h"

#if !defined( CFG_ARGS_EEPROM_ADDR )
#   error Please define CFG_ARGS_EEPROM_ADDR in Config.h
#endif

// ---- Public Variables ----------------------------------------------------

// ---- Private Constants and Types -----------------------------------------

// ---- Private Function Prototypes -----------------------------------------

// ---- Functions -----------------------------------------------------------

/***************************************************************************/
/**
*  Retrieves the number of arguments stored in EEPROM
*/

uint8_t NumArgs( void )
{
    uint8_t numArgs = eeprom_read_byte( (uint8_t *)CFG_ARGS_EEPROM_ADDR );
    if ( numArgs == 0xFF )
    {
        numArgs = 0;
    }
    return numArgs;

} // NumArgs

/***************************************************************************/
/**
*  Retrieves an argument from EEPROM. Returns the length of the string
*  copied into the buffer.
*/

uint8_t GetArg( uint8_t argIdx, char *argBuf, uint8_t bufLen )
{
    uint8_t numArgs = NumArgs();
    uint8_t argOffset;
    uint8_t bytesCopied;

    if (( argIdx > numArgs ) || ( bufLen < 2 ))
    {
        *argBuf = '\0';
        return 0;
    }
    argOffset = eeprom_read_byte( (uint8_t *)CFG_ARGS_EEPROM_ADDR + argIdx + 1 );

    bytesCopied = 0;

    while ( bytesCopied < bufLen )
    {
        uint8_t ch = eeprom_read_byte( (uint8_t *)CFG_ARGS_EEPROM_ADDR + argOffset );

        argBuf[ bytesCopied ] = ch;

        if ( ch == '\0' )
        {
            break;
        }

        argOffset++;
        bytesCopied++;
    }

    if ( bytesCopied == bufLen )
    {
        bytesCopied--;
        argBuf[ bytesCopied ] = '\0';
    }

    return bytesCopied;

} // GetArg

