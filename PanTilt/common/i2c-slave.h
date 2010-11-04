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
*   @file   i2c-slave.h
*
*   @brief  Contains definitions for the slave portion of the i2c support.
*
****************************************************************************/

#if !defined( I2C_SLAVE_H )
#define I2C_SLAVE_H         /**< Include Guard                             */

/* ---- Include Files ---------------------------------------------------- */

#include <string.h>
#include <avr/io.h>

#include "i2c.h"
#include "Config.h"
#include "Log.h"

/**
 *  @addtogroup I2C
 *  @{
 */

/* ---- Constants and Types ---------------------------------------------- */

typedef enum
{
    I2C_STATE_IDLE,         ///< No transaction currently in progress
    I2C_STATE_MASTER_TX,    ///< Transmitting as Master
    I2C_STATE_MASTER_RX,    ///< Receiving as Master
    I2C_STATE_SLAVE_TX,     ///< Transmitting as Slave
    I2C_STATE_SLAVE_RX,     ///< Receiving as Slave
    I2C_STATE_SLAVE_WRITE,  ///< Write processed
    I2C_STATE_UNINITIALIZED ///< I2C_Init hasn't been called yet

} I2C_State_t;

/**
 *  Callback which is called when a slave command is received.
 */

typedef int (*I2C_ProcessCommand)( I2C_Data_t *rcvdPacket );

/**
 *  Defines the version of I2C_Globals_t struct. This includes the order
 *  of the variables, along with any semantics associated with the variable.
 * 
 *  ANY change to the structure requires the version number to be incremented.
 */

#define I2C_GLOBALS_VERSION     1

/**
 *  The min version, determines the minimum version of this structure that
 *  the current structure is compatable with.
 */

#define I2C_MIN_GLOBALS_VERSION 1

/**
 *  The bootloader can use any old RAM it wants. The normal code has to 
 *  get it out of the .data section. So we collect it all together in a
 *  structure so it can be shared.
 */

typedef struct
{
    uint8_t         m_structSize;       ///< Size of this structure in bytes
    uint8_t         m_version;          ///< Version number of this structure
    uint8_t         m_minVersion;       ///< Minimum compatible version of this structure

    volatile I2C_State_t m_state;       ///< Current state of the protocol

    I2C_ProcessCommand  m_processCommand;

    I2C_Addr_t      m_deviceAddrRW;     ///< 7 bit address (shifted left by 1 ) + R/W bit

#if CFG_I2C_USE_CRC
    uint8_t         m_useCrc;           ///< Is this transaction using CRC's?
    uint8_t         m_crc;              ///< CRC being calculated
#endif

    uint8_t        *m_readData;         ///< Ptr to read buffer
    uint8_t         m_readDataLen;      ///< Length of data we're expecting
    uint8_t         m_readDataValid;    ///< Do we have a valid packet?

    uint8_t        *m_writeData;        ///< Ptr to write buffer
    uint8_t         m_writeDataLen;     ///< Length of data to put in the write buffer
    uint8_t         m_writeDataIdx;     ///< Next byte to write comes from m_writeData[ m_writeDataIdx ]

    I2C_Data_t      m_slavePacket;      ///< Packet buffer used for slave transactions.

} I2C_Globals_t;

/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */

#if !defined( I2C_LOG_ENABLED )
#   if STANDALONE
#       define I2C_LOG_ENABLED 0
#   else
#       define I2C_LOG_ENABLED 0
#   endif
#endif

#if I2C_LOG_ENABLED
#   if CFG_LOG_TO_BUFFER
#       define  I2C_LOG0( fmt )             LogBuf0( "I2C: " fmt )
#       define  I2C_LOG1( fmt, arg1 )       LogBuf1( "I2C: " fmt, arg1 )
#       define  I2C_LOG2( fmt, arg1, arg2 ) LogBuf2( "I2C: " fmt, arg1, arg2 )
#   else
#       define  I2C_LOG0( fmt )             Log( "I2C: " fmt )
#       define  I2C_LOG1( fmt, arg1 )       Log( "I2C: " fmt, arg1 )
#       define  I2C_LOG2( fmt, arg1, arg2 ) Log( "I2C: " fmt, arg1, arg2 )
#   endif
#else
#   define  I2C_LOG0( fmt )
#   define  I2C_LOG1( fmt, arg1 )
#   define  I2C_LOG2( fmt, arg1, arg2 )
#endif

uint8_t I2C_SlaveHandler( I2C_Globals_t *globals );

#if defined( __AVR_ATmega8__ ) \
 || defined (__AVR_ATmega48__) \
 || defined (__AVR_ATmega88__) \
 || defined (__AVR_ATmega168__)

#define I2C_PORT        PORTC
#define I2C_DDR         DDRC
#define I2C_SCL_MASK    ( 1 << 5 )
#define I2C_SDA_MASK    ( 1 << 4 )

#elif defined( __AVR_ATmega16__ ) \
   || defined( __AVR_ATmega32__ )

#define I2C_PORT        PORTC
#define I2C_DDR         DDRC
#define I2C_SCL_MASK    ( 1 << 0 )
#define I2C_SDA_MASK    ( 1 << 1 )

#elif defined (__AVR_ATmega64__) \
   || defined (__AVR_ATmega128__)

#define I2C_PORT        PORTD
#define I2C_DDR         DDRD
#define I2C_SCL_MASK    ( 1 << 0 )
#define I2C_SDA_MASK    ( 1 << 1 )

#else
#   error Common/avr/i2c-slave.h Processor not supported
#endif

/****************************************************************************/
/**
*   Initializes the I2C module. This function is declared inline since
*   it's only called once anyways, but the real reason is that we want
*   to guarantee that it matchcs up with the I2C_Globals_t structure.
*/

static inline void I2C_SlaveInit( I2C_Globals_t *globals, I2C_Addr_t slaveAddr, I2C_ProcessCommand processCmd )
{
    I2C_LOG1( "Init Address: 0x%02x\n", slaveAddr );

    memset( (void *)globals, 0, sizeof( *globals ));

    globals->m_structSize   = sizeof( *globals );

    globals->m_version      = I2C_GLOBALS_VERSION;
    globals->m_minVersion   = I2C_MIN_GLOBALS_VERSION;

    globals->m_state        = I2C_STATE_IDLE;

    globals->m_processCommand = processCmd;

    // Currently, we don't recognize General Call addresses

#if defined( CFG_I2C_LED_PORT )
    CFG_I2C_LED_DDR |= CFG_I2C_LED_MASK;
#endif

    I2C_DDR &= ~( I2C_SCL_MASK | I2C_SDA_MASK );

#if CFG_I2C_INTERNAL_PULLUPS
    I2C_PORT |= ( I2C_SCL_MASK | I2C_SDA_MASK );
#else
    I2C_PORT &= ~( I2C_SCL_MASK | I2C_SDA_MASK );
#endif

    // Enable I2C

    TWBR = 32;

    TWCR |= ( 1 << TWEN );
    TWCR |= ( 1 << TWINT );
    TWCR |= ( 1 << TWIE );
    TWCR |= ( 1 << TWEA );

    TWAR = slaveAddr << 1;

} // I2C_SlaveInit

/** @} */

#endif /* I2C_SLAVE_H */

