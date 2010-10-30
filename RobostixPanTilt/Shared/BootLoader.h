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
*   @file   BootLoader.h
*
*   @brief  Contains the definitions used to communicate with the 
*           Bootloder.
*
****************************************************************************/
/**
*   @defgroup   BootLoader   I2C BootLoader
*
*   @brief      BootLoader for downloading programs to an AVR via
*               the I2C bus.
*
****************************************************************************/

#if !defined( BOOTLOADER_H )
#define BOOTLOADER_H                   ///< Include Guard

/* ---- Include Files ---------------------------------------------------- */

#include <inttypes.h>

#if defined( AVR )
#include <avr/io.h>
#endif 

#include "i2c.h"

/**
 *  @addtogroup BootLoader
 *  @{
 */

/* ---- Constants and Types ---------------------------------------------- */

typedef union
{
    uint32_t    val32;
    uint16_t    val16;
    uint8_t     byte[ 4 ];

} BL_Addr_t;

#define BL_ADDR_MASK    0x00FFFFFFuL

//---------------------------------------------------------------------------
/**
 *  Defines the version of this API. This includes the layout of the 
 *  various structures, along with the semantics associated with the 
 *  protocol. Any changes require the version number to be incremented.
 */

#define BL_API_VERSION      1

//---------------------------------------------------------------------------
/**
 *  The min version, determines the minimum version that this API is
 *  compatable with. This allows old host side programs to determine
 *  that they're not compatible.
 */

#define BL_API_MIN_VERSION  1

//---------------------------------------------------------------------------
/**
 *  Maximum number of data bytes that can be trasnferred in the read/write
 *  commands. In order to remain compatible with SMBus, the maximum packet
 *  size if 32 bytes. We subtract 4 for the address that is passed.
 */

#define BL_MAX_DATA_BYTES   28

#define BL_REG_BASE             0xB0

//---------------------------------------------------------------------------
/**
 *  Casues the bootloader to return information about the device.
 *
 *  Read-Block
 *
 *  Returns:    BootLoaderInfo_t
 */

#define BL_REG_GET_INFO         0xB0

//---------------------------------------------------------------------------
/**
 *  Returns information in response to the BL_CMD_GET_INFO command. This
 * includes version information about the BootLoader, and some size 
 * information about the various memories.
 */

typedef struct
{
    I2C_Addr_t  devAddr;        ///< The i2c address of the device.
    uint8_t     version;        ///< API Version the device is using.
    uint8_t     minVersion;     ///< Minimum API version the device is compatible with.
    uint8_t     pad[ 3 ];       ///< Reserved - set to zero.
    uint16_t    partNumber;     ///< Part number (bytes 2 & 3 of signature)
    uint16_t    regSize;        ///< Size of the register area, in bytes.
    uint16_t    ramSize;        ///< Size of the SRAM area, in bytes.
    uint16_t    eepromSize;     ///< Size of the EEPROM, in bytes.
    uint16_t    spmPageSize;    ///< Size of the SPM Page Size.
    uint32_t    flashSize;      ///< Size of the flash, in bytes.

} BootLoaderInfo_t;

//---------------------------------------------------------------------------
/**
 *  The BL_MEM_TYPE_XXX constants can be combined with the high byte of the
 *  address when performing a read or write operation.
 */

#define BL_MEM_TYPE_MASK    0xC0
#define BL_MEM_TYPE_FLASH   0x00
#define BL_MEM_TYPE_SRAM    0x40
#define BL_MEM_TYPE_EEPROM  0x80

//---------------------------------------------------------------------------
/**
 *  Reads data from memory.
 *
 *  Process-Call-Block
 *
 *  Param:      number of bytes to read
 *  Returns:    Data that was read
 */

#define BL_REG_READ_DATA            0xB1

typedef struct
{
    BL_Addr_t   addr;
    uint8_t     len;
    uint8_t     pad[ 3 ];

} BootLoaderRead_t;

//---------------------------------------------------------------------------
/**
 *  Writes data to memory.
 *
 *  Write-Block
 *
 *  Param:      Data to write
 */

#define BL_REG_WRITE_DATA           0xB2

typedef struct
{
    BL_Addr_t   addr;
    uint8_t     data[ BL_MAX_DATA_BYTES ];

} BootLoaderWrite_t;

//---------------------------------------------------------------------------
/**
 *  Executes the application program which has been downloaded.
 * 
 *  Param:      null terminated string
 */

#define BL_CMD_RUN_APP              0xB3

//---------------------------------------------------------------------------
/**
 *  Causes the bootloader to "reset" itself. This is useful to cause the
 *  an updated address to be used from EEPROM.
 * 
 *  Param:      None
 */

#define BL_CMD_REBOOT               0xB4

//---------------------------------------------------------------------------
/**
 *  Describes the amount and layout of the EEPROM data required by the 
 *  bootloader. This will be located at the top end of EEPROM.
 * 
 *  IMPORTANT: When adding data to this structure, add it to the beginning
 *             of the structure, rather than at the end. This is because
 *             it's pinned at the end of EEPROM and by growing into lower
 *  memory, we don't necessarily have to reload the EEPROM.
 */

#define BL_NODE_NAME_LEN    16

typedef struct
{
    char        name[ BL_NODE_NAME_LEN ];   ///< Name of this node (null terminated)
    uint8_t     pad;            ///< Reserved - set to zero
    uint8_t     bootDelay;      ///< # seconds to wait for boot command
    uint8_t     i2cAddr;        ///< i2c address (in lower 7 bits format)
    uint8_t     structSize;     ///< Size of this structure, in bytes.

} BootLoaderEepromData_t;

//---------------------------------------------------------------------------
/**
 *  The default i2c address to use when none is cofigured. Once the device
 *  is recognized, the master can download a new address by updating 
 *  the EEPROM.
 */

#define BL_DEFAULT_I2C_ADDR         0x0B

//---------------------------------------------------------------------------
/**
 *  The default amount of time to wait for a boot loader command before
 *  executing the currently loaded application.
 */

#define BL_DEFAULT_BOOT_DELAY       5

//---------------------------------------------------------------------------
/*
 * Test commands
 */

#define BL_TEST_CMDS    1

#if BL_TEST_CMDS

#define BL_CMD_TEST_READ    0xC0
#define BL_CMD_TEST_WRITE   0xC1
#define BL_CMD_TEST_CALL    0xC2
#define BL_CMD_TEST_NOTIFY  0xCC

#endif

#if defined( AVR )

/**
 *  This structure defines variables which are required if the bootloader
 *  functionality is to be used as part of the application program.
 */

typedef struct
{
    uint32_t    m_pageAddress;      ///< Address of data currently in page buffer
    uint8_t     m_pageDirty;        ///< Does the page buffer contain unwritten data?
    uint8_t     m_pageBuf[ SPM_PAGESIZE ];

} BootLoaderGlobals_t;

/**
 *  This structure defines the jump table stored in flash
 *
 */

#endif  // AVR

/* ---- Variable Externs ------------------------------------------------- */

/**
 *  Description of variable.
 */

/* ---- Function Prototypes ---------------------------------------------- */

/*
 *  Just include prototypes here. Put full descriptions in the .c files.
 */

/** @} */

#endif /* BOOTLOADER_H */

