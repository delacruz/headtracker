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
*   @file   i2c-slave-boot.c 
*
*   @brief  Thie file implements the slave portion of the i2c support
*           by "borrowing" the code from the boot-loader.
*
*****************************************************************************/

/* ---- Include Files ----------------------------------------------------- */

#include <avr/io.h>

#include "BootLoader.h"
#include "i2c-slave.h"
#include "i2c-slave-boot.h"

/* ---- Public Variables -------------------------------------------------- */

/* ---- Private Constants and Types --------------------------------------- */

// The i2c bootloader currently occupies 2K of space. We try to reserve 4K
// to allow for a bootloader "reloader" which would be able to re-program
// the bootloader itself.

#if defined( __AVR_ATmega8__ ) \
 || defined (__AVR_ATmega48__) \
 || defined (__AVR_ATmega88__)

// These devices only have a 2k boot area, so we expect the bootloader to start
// 2K back from the end of the flash area.

#define BL_START        ( FLASHEND - 2048 + 1 )
#define BL_MAGIC_ADDR   ( BL_START + 2 )            // Only 2 bytes/vector

#elif defined (__AVR_ATmega168__) \
   || defined ( __AVR_ATmega16__ )

// These devices only have a 2k boot area, so we expect the bootloader to start
// 2K back from the end of the flash area.

#define BL_START        ( FLASHEND - 2048 + 1 )
#define BL_MAGIC_ADDR   ( BL_START + 4 )

#elif defined( __AVR_ATmega32__ ) \
   || defined (__AVR_ATmega64__) \
   || defined (__AVR_ATmega128__)

// These devices have 4K or larger boot area, so we expect the bootloader to
// start 4K back from the end of the flash area.

#define BL_START        ( FLASHEND - 4096 + 1 )
#define BL_MAGIC_ADDR   ( BL_START + 4 )

#else
#   error   Common/i2c-slave-boot.h Processor not supported
#endif

#define BL_MAGIC    0xB007

#if defined( RAMPZ )

// These devices have more than 64K so we need to use extended addresses

   typedef uint32_t    PgmAddr_t;

#   define  PGM_READ_WORD   pgm_read_word_far

   // From memcpy_EP.S

    void *memcpy_EP( void *dst, uint32_t src, size_t len );

#else

    typedef uint16_t    PgmAddr_t;

#   define  PGM_READ_WORD   pgm_read_word_near

#   define  memcpy_EP( dst, src, len )  memcpy_P( dst, (PGM_VOID_P)(uint16_t)src, len )

#endif

typedef struct
{
    void    (*AppBoot)( void );
    uint8_t (*I2C_SlaveHandler)( I2C_Globals_t *globals );
    int     (*BootLoaderProcessCommand)( BootLoaderGlobals_t *globals, I2C_Data_t *packet );

} BootLoaderJumpTable;

/* ---- Private Variables ------------------------------------------------- */

BootLoaderGlobals_t     gBootLoaderGlobals;
I2C_Globals_t           gI2cGlobals;
BootLoaderJumpTable     gJumpTable;

/* ---- Private Function Prototypes --------------------------------------- */

/* ---- Functions --------------------------------------------------------- */

//***************************************************************************
/**
*   AppBoot - Enters the BootLoader
*/

void AppBoot( void )
{
    gJumpTable.AppBoot();

} // AppBoot

//***************************************************************************
/**
*   I2C Interrupt service routine
*/

uint8_t I2C_SlaveBootHandler( void )
{
    return gJumpTable.I2C_SlaveHandler( &gI2cGlobals );

} // I2C_SlaveHandler

//***************************************************************************
/**
*   Initializes the I2C module in such a way that the code from the bootloader
*   is utilized.
*
*   The I2C slave address uses the same slave address that the bootloader has
*   been configured with.
*/

uint8_t I2C_SlaveBootInit( I2C_ProcessCommand processCmd )
{
    PgmAddr_t   addr;
    uint16_t    magic;
    uint16_t    numEntries;

    // Locate the bootloader

    addr = BL_MAGIC_ADDR;

    magic = PGM_READ_WORD( addr );
    if ( magic != BL_MAGIC )
    {
        LogError( "Expecting magic 0x%04x, found 0x%04x @ 0x%05lx\n", BL_MAGIC, magic, addr );
        return 0;
    }
    addr += 2;

    // The word after the magic number is the address of the jump table

    addr = PGM_READ_WORD( addr );
    addr <<= 1;

    numEntries = PGM_READ_WORD( addr );
    Log( "numEntries = %d\n", numEntries );

    if ( numEntries < ( sizeof( gJumpTable ) / 2 ))
    {
        return 0;
    }
    addr += 2;
    memcpy_EP( &gJumpTable, addr, sizeof( gJumpTable ));

    Log( "AppBoot                  = 0x%04x\n", gJumpTable.AppBoot );
    Log( "I2C_SlaveHandler         = 0x%04x\n", gJumpTable.I2C_SlaveHandler );
    Log( "BootLoaderProcessCommand = 0x%04x\n", gJumpTable.BootLoaderProcessCommand );

    // Since we're integrating with the bootloader, we make the assumption
    // that the bootloader has already initialized the TWAR register.

    I2C_SlaveInit( &gI2cGlobals, TWAR >> 1, processCmd );

    return 1;

} // I2C_SlaveBootInit

//***************************************************************************
/**
*   BootLoaderProcessCommand - Processes a BootLoader i2c command
*/

int I2C_SlaveBootProcessCommand( I2C_Data_t *packet )
{
    return gJumpTable.BootLoaderProcessCommand( &gBootLoaderGlobals, packet );

} // BootLoaderProcessCommand


