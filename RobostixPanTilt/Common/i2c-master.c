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
*   @file   i2c-slave.c 
*
*   @brief  Implements the slave portion of the i2c support.
*
*****************************************************************************/

/* ---- Include Files ----------------------------------------------------- */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <compat/twi.h>

#include "i2c-slave.h"
#include "Crc8.h"
#include "Log.h"
#include "Config.h"

/* ---- Public Variables -------------------------------------------------- */
/* ---- Private Constants and Types --------------------------------------- */

#define MAX_RETRIES     10

/* ---- Private Variables ------------------------------------------------- */

static  I2C_Globals_t   gGlobals;

/**
 *  We set this flag to indicate that we got some type of failure for which
 *  we need to redo our transaction (arbitration lost, unexpected NACK)
 */

static  uint8_t     gRetryRequired;
static  I2C_Error_t gErrorCode;

/* ---- Private Function Prototypes --------------------------------------- */

static  int ProcessCommand( I2C_Data_t *rcvdPacket );

/* ---- Functions --------------------------------------------------------- */

/**
 * @addtogroup I2C
 * @{
 */

//***************************************************************************
/**
*   Determines if the i2c module is currently idle or not.
*/

inline int I2C_IsIdle( void )
{
    return gGlobals.m_state == I2C_STATE_IDLE;

} // I2C_SendByte

//***************************************************************************
/**
*   Sends a single character out the i2c interface.
*/

inline void I2C_SendByte( uint8_t dataByte )
{
    I2C_LOG1( "SendByte: 0x%02x\n", dataByte );

    TWDR = dataByte;

} // I2C_SendByte

//***************************************************************************
/**
*   Sends a start condition onto the bus. Note, that if the bus is busy
*   this could take a while before it actually happens. Once the START
*   has been asserted, a TW_START pr TW_REP_START status will be sent to
*   the interrupt handler.
*/

inline void I2C_SendStart( void )
{
    I2C_LOG0( "SendStart\n" );

    TWCR |= (( 1 << TWSTA ) | ( 1 << TWEA ));

} // I2C_SendStart

//***************************************************************************
/**
*   Sends a stop condition onto the bus.
*/

inline void I2C_SendStop( void )
{
    I2C_LOG0( "SendStop\n" );

    // Leave TWEA on so we can receive as a slave

    TWCR |= (( 1 << TWSTO ) | ( 1 << TWEA ));

} // I2C_SendStop

//***************************************************************************
/**
*   Waits for the i2c module to enter an IDLE state 
*/

inline void I2C_WaitForIdle( void )
{
    I2C_LOG0( "WaitForIdle\n" );

    while ( !I2C_IsIdle() )
    {
        LogBufDump();
    }
    I2C_LOG0( "WaitForIdle finished\n" );

} // I2C_WaitForIdle

//***************************************************************************
/**
*   I2C Interrupt service routine
*
*/

SIGNAL(SIG_2WIRE_SERIAL)
{
    // The TWI handling is basically a state machine driven by the status
    // bits presented by the hardware.

	uint8_t status = TW_STATUS;
    uint8_t twcr   = TWCR;

	switch( status )
	{
        case TW_BUS_ERROR:  // 0x00 - Bus error due to illegal start or stop condition
        {
            I2C_LOG0( "BUS-ERROR\n" );

            gErrorCode = I2C_ERROR_ADDR_NACK;
            gGlobals.m_state = I2C_STATE_IDLE;
            break;
        }

        // ----- Master Common States -----

        case TW_START:      // 0x08 - A START condition has been transmitted
        case TW_REP_START:  // 0x10 - A repeated START condition has been transmitted.
        {
            if ( status == TW_START )
            {
                I2C_LOG2( "START 0x%02x:%c\n", 
                          gGlobals.m_deviceAddrRW >> 1, 
                          gGlobals.m_deviceAddrRW & TW_READ ? 'R' : 'W' );
            }
            else
            {
                I2C_LOG2( "REP-START 0x%02x:%c\n", 
                          gGlobals.m_deviceAddrRW >> 1, 
                          gGlobals.m_deviceAddrRW & TW_READ ? 'R' : 'W' );
            }

            // Send the SLA+R or SLA+W

            I2C_SendByte( gGlobals.m_deviceAddrRW );
            gGlobals.m_crc = Crc8( gGlobals.m_crc, gGlobals.m_deviceAddrRW );

            // Clear the start condition

            twcr &= ~( 1 << TWSTA );

            break;
        }

        // ----- Master Transmitter States -----

        case TW_MT_SLA_ACK:     // 0x18 - SLA+W has been transmitted, ACK has been received
        case TW_MT_DATA_ACK:    // 0x28 - Data byte has been transmitted, ACK has been received
        {
            if ( status == TW_MT_SLA_ACK )
            {
                I2C_LOG0( "MT-SLA-ACK\n" );
            }
            else
            {
                I2C_LOG0( "MT-DATA-ACK\n" );
            }

            if ( gGlobals.m_writeDataIdx < gGlobals.m_writeDataLen )
            {
                I2C_SendByte( gGlobals.m_writeData[ gGlobals.m_writeDataIdx ]);
                gGlobals.m_crc = Crc8( gGlobals.m_crc, gGlobals.m_writeData[ gGlobals.m_writeDataIdx ]);
                gGlobals.m_writeDataIdx++;
            }
            else
            if ( gGlobals.m_readData == NULL )
            {
                // This is a write transaction. Send the CRC, if required

                if (( gGlobals.m_writeDataIdx == gGlobals.m_writeDataLen ) && gGlobals.m_useCrc )
                {
                    gGlobals.m_writeDataIdx++;

                    I2C_LOG1( "Sending CRC: 0x%02x\n", gGlobals.m_crc );
                    I2C_SendByte( gGlobals.m_crc );
                }
                else
                {
                    I2C_LOG0( "SendStop\n" );
                    twcr |= ( 1 << TWSTO );
                    gGlobals.m_state = I2C_STATE_IDLE;
                }
            }
            else
            {
                // This is a Read or Call transaction. Send a repeated Start

                I2C_LOG0( "Send RepeatedStart\n" );
                twcr |= ( 1 << TWSTA );

                gGlobals.m_deviceAddrRW |= TW_READ;
            }
            break;
        }


        case TW_MT_SLA_NACK:    // 0x20 - SLA+x has been transmitted, no response received
        case TW_MT_DATA_NACK:   // 0x30 - Data has been transmitted, no response received
        {
            if ( status == TW_MT_SLA_NACK )
            {
                Log( "MT-SLA-NACK\n" );
                gErrorCode = I2C_ERROR_ADDR_NACK;
            }
            else
            {
                Log( "MT-DATA-NACK\n" );
                gErrorCode = I2C_ERROR_DATA_NACK;
            }

            I2C_LOG0( "SendStop\n" );
            twcr |= ( 1 << TWSTO );

            gRetryRequired = 1;
            gGlobals.m_state = I2C_STATE_IDLE;
            break;
        }

        case TW_MT_ARB_LOST:    // 0x38 - Arbitration lost (same code as TW_MR_ARB_LOST)
        {
            I2C_LOG0( "ARB-LOST\n" );

            gRetryRequired = 1;
            gErrorCode = I2C_ERROR_ARBITRATION_LOST;
            gGlobals.m_state = I2C_STATE_IDLE;
            break;
        }

        // ----- Master Receiver States -----

        case TW_MR_SLA_ACK:		// 0x40 - SLA+R has been transmitted, ACK has been received
        {
            I2C_LOG0( "MR-SLA-ACK\n" );

            if (( gGlobals.m_readDataLen == 0 ) || (( gGlobals.m_readDataIdx + 1 ) < gGlobals.m_readDataLen ))
            {
                twcr |= ( 1 << TWEA );
            }
            else
            {
                // The fisrt data byte will also be the last, we'll NACK it

                twcr &= ~( 1 << TWEA );
            }
            break;
        }

        case TW_MR_SLA_NACK:	// 0x48 - SLA+R has been transmitted, no response received
        {
            I2C_LOG0( "MR-SLA-NACK\n" );

            I2C_LOG0( "SendStop\n" );
            twcr |= ( 1 << TWSTO );

            gRetryRequired = 1;
            gErrorCode = I2C_ERROR_ADDR_NACK;
            gGlobals.m_state = I2C_STATE_IDLE;
            break;
        }

        case TW_MR_DATA_ACK:	// 0x50 - data has been received, ACK has been transmitted
        {
            uint8_t data;

            data = TWDR;

            I2C_LOG1( "MR-DATA-ACK: 0x%02x\n", data );

            gGlobals.m_readData[ gGlobals.m_readDataIdx ] = data;

            if (( gGlobals.m_readDataLen == 0 ) && ( gGlobals.m_readDataIdx == 0 ))
            {
                // We just received the length, set it up properly.

                if ( data > I2C_MAX_DATA_LEN )
                {
                    // Something is VERY screwed up.

                    I2C_LOG1( "Rcvd Data Len too big: 0x%02x\n",  data );

                    I2C_LOG0( "SendStop\n" );
                    twcr |= ( 1 << TWSTO );

                    gRetryRequired = 1;
                    gErrorCode = I2C_ERROR_BAD_LEN;
                    gGlobals.m_state = I2C_STATE_IDLE;
                    break;
                }
                I2C_LOG1( "Rcvd data len of %d\n", data );

                gGlobals.m_readDataLen = data + 1;  // + 1 for len

                I2C_LOG1( "Setting m_readDataLen to %d\n", gGlobals.m_readDataLen );
            }

            gGlobals.m_readDataIdx++;   // Idx now = number of chars rcvd

            if (( gGlobals.m_readDataIdx < gGlobals.m_readDataLen ) || gGlobals.m_useCrc )
            {
                // Due to the way the HW works, we have to decide BEFORE
                // receiving the next byte if we want to ACK it or not
                // which means that we'll alwys ACK the CRC and if it
                // fails, we'll retry the read.

                twcr |= ( 1 << TWEA );

                if ( gGlobals.m_readDataIdx > gGlobals.m_readDataLen )
                {
                    // We've received the CRC

                    if ( gGlobals.m_crc != data )
                    {
                        I2C_LOG2( "CRC failed - Rcvd 0x%02x, expecting 0x%02x\n", data,  gGlobals.m_crc );
                        gErrorCode = I2C_ERROR_BAD_CRC;
                        gRetryRequired = 1;
                    }
                    else
                    {
                        I2C_LOG0( "CRC passed\n" );
                    }

                    I2C_LOG0( "SendStop\n" );
                    twcr |= ( 1 << TWSTO );

                    gGlobals.m_state = I2C_STATE_IDLE;
                }
            }
            else
            {
                // The next data byte will be the last one, so NACK it

                twcr &= ~( 1 << TWEA );
            }
            gGlobals.m_crc = Crc8( gGlobals.m_crc, data );
            break;
        }

        case TW_MR_DATA_NACK:	// 0x58 - data has been received, NACK has been send
        {
            I2C_LOG0( "MR-DATA-NACK\n" );

            // This is the normal situation for receiving the last byte
            // when PEC isn't being used.

            I2C_LOG0( "SendStop\n" );
            twcr |= ( 1 << TWSTO );

            gGlobals.m_state = I2C_STATE_IDLE;
            break;
        }

        default:
        {
            if ( !I2C_SlaveHandler( &gGlobals ))
            {
                I2C_LOG1( "Unrecognized status: 0x%x\n", status );

                twcr &= ~( 1 << TWEA );
            }

            break;
        }
    }

    // Now that we've finished dealing with the interrupt, set the TWINT flag
    // which will stop the clock stretching and clear the interrupt source

    TWCR = twcr | ( 1 << TWINT );

} // SIG_2WIRE_SERIAL

/****************************************************************************/
/**
*   Initializes the I2C module.
*/

void I2C_Init( I2C_Addr_t addr )
{
    I2C_SlaveInit( &gGlobals, addr, ProcessCommand );

    // The SCL grequency is determined using the following formula:
    //
    //        CPU-Clock-Frequency
    //  --------------------------------
    //  16 + ( 2 * TWBR * ( 4 ^ TWPS ))

    // Set TWPS to 0, so 4 ^ TWPS = 1.

    TWSR &= ~(( 1 << TWPS0 ) | ( 1 << TWPS1 ));

    // So we get:
    //
    //  TWBR = ( CPU-Freq / ( 2 * SCL-Freq )) - 8

#if !defined( CFG_I2C_CLOCK )
#   error CFG_I2C_CLOCK is not defined
#endif

    TWBR = ( CFG_CPU_CLOCK / ( 2 * CFG_I2C_CLOCK )) - 8;

} // I2C_Init

/****************************************************************************/
/**
*    Issues an I2C Call transaction, which consists of a write of some
*   arbitrary amount of data followed by a read of some data.~
*/

I2C_Error_t I2C_MasterCall( I2C_Addr_t slaveAddr, const I2C_Data_t *writeData, I2C_Data_t *readData )
{
    uint8_t     sreg;
    uint8_t     retryCount;
    uint8_t     readLen;
    I2C_Error_t rc;

    //  Does a Write/Read (aka Process Call) with an i2c slave
    //
    //  On the i2c bus, the following will take place (M=Master, S=Slave):
    //
    //  M->S:   Start
    //  M->S:   slave-addr w/ W
    //  M->S:   command
    //  M->S:   len
    //  M->S:   data (first byte should be address of master)
    //  M->S:   Repeated Start
    //  M->S:   slave-addr w/ R
    //  S->M:   len
    //  S->M:   data (first byte should be address of slave)
    //  S->M:   CRC
    //  M->S:   Stop

    // Wait for the i2c to become idle

    I2C_WaitForIdle();
    retryCount = 0;
    readLen = readData->m_len;

    do
    {
        I2C_LOG2( "I2C_MasterCall: Addr: 0x%02x Attempt: %d\n", slaveAddr, retryCount );

        sreg = SREG;    
        cli();
        {
            // Test again with interrupts disabled so we know it's still
            // really idle. Somebody could have addressed us right after
            // our check for IDLE.

            if ( gGlobals.m_state == I2C_STATE_IDLE )
            {
                gGlobals.m_state = I2C_STATE_MASTER_TX;

                gRetryRequired = 0;
                gErrorCode = I2C_ERROR_NONE;

                gGlobals.m_useCrc = (( slaveAddr & I2C_USE_CRC ) != 0 );

                gGlobals.m_deviceAddrRW = ( slaveAddr << 1 ) | TW_WRITE;
                gGlobals.m_writeData    = (char *)&writeData->m_command;
                gGlobals.m_writeDataLen = writeData->m_len + 2;   // +1 for cmd, +1 for len
                gGlobals.m_writeDataIdx = 0;

                readData->m_command = writeData->m_command;
                gGlobals.m_readData = &readData->m_len;
                gGlobals.m_readDataLen = 0;   // Calculated by interrupt handler when len is received
                gGlobals.m_readDataIdx = 0;

                gGlobals.m_crc = 0;

                // Kick things off by sending out a START

                I2C_SendStart();
                retryCount++;
            }
            else
            {
                // Some other type of transaction just started. Loop and
                // try again.

                gErrorCode = I2C_ERROR_ARBITRATION_LOST;
                gRetryRequired = 1;
            }
        }
        SREG = sreg;

        // Wait for the transaction to complete. Otherwise we'll never know
        // if it actually happened or not. If we lose arbitration, then we
        // need to re-issue the write.

        I2C_WaitForIdle();
        rc = gErrorCode;

    } while ( gRetryRequired && ( retryCount < MAX_RETRIES ));

    if ( retryCount > 1 )
    {
        Log( "I2C_MasterCall: took %d attempts\n", retryCount );
    }
    return rc;

} // I2C_MasterCall

/****************************************************************************/
/**
*    Issues an I2C Read transaction, which consists of sending a command
*   followed by reading a response.
*/

uint8_t I2C_MasterRead( I2C_Addr_t slaveAddr, I2C_Data_t *readData )
{
    uint8_t     sreg;
    uint8_t     retryCount;
    uint8_t     readLen;
    I2C_Error_t rc;

    //  On the i2c bus, the following will take place (M=Master, S=Slave):
    //
    //  M->S:   Start
    //  M->S:   slave-addr w/ W
    //  M->S:   command
    //  M->S:   Repeated Start
    //  M->S:   slave-addr w/ R
    //  S->M:   len
    //  S->M:   data (first byte should be address of slave)
    //  S->M:   CRC
    //  M->S:   Stop

    // Wait for the i2c to become idle

    I2C_WaitForIdle();
    retryCount = 0;
    readLen = readData->m_len;

    do
    {
        I2C_LOG2( "I2C_MasterRead: Addr: 0x%02x Attempt: %d\n", slaveAddr, retryCount );

        sreg = SREG;    
        cli();
        {
            // Test again with interrupts disabled so we know it's still
            // really idle. Somebody could have addressed us right after
            // our check for IDLE.

            if ( gGlobals.m_state == I2C_STATE_IDLE )
            {
                gGlobals.m_state = I2C_STATE_MASTER_RX;

                gRetryRequired = 0;
                gErrorCode = I2C_ERROR_NONE;

                gGlobals.m_useCrc = (( slaveAddr & I2C_USE_CRC ) != 0 );

                gGlobals.m_deviceAddrRW = ( slaveAddr << 1 ) | TW_WRITE;  // We start off by writing the command
                gGlobals.m_writeData    = &readData->m_command;
                gGlobals.m_writeDataLen = 1;
                gGlobals.m_writeDataIdx = 0;

                gGlobals.m_readData = &readData->m_len;
                gGlobals.m_readDataLen = 0;   // Calculated by interrupt handler when len is received
                gGlobals.m_readDataIdx = 0;

                gGlobals.m_crc = 0;

                // Kick things off by sending out a START

                I2C_SendStart();
                retryCount++;

            }
            else
            {
                // Some other type of transaction just started. Loop and
                // try again.

                gErrorCode = I2C_ERROR_ARBITRATION_LOST;
                gRetryRequired = 1;
            }
        }
        SREG = sreg;

        // Wait for the transaction to complete. Otherwise we'll never know
        // if it actually happened or not. If we lose arbitration, then we
        // need to re-issue the write.

        I2C_WaitForIdle();
        rc = gErrorCode;

    } while ( gRetryRequired && ( retryCount < MAX_RETRIES ));

    if ( retryCount > 1 )
    {
        Log( "I2C_MasterRead: took %d attempts\n", retryCount );
    }

    return rc;

} // I2C_MasterRead

/****************************************************************************/
/**
*    Issues an I2C Write transaction, which consists of sending a command
*   followed by some data.
*/

uint8_t I2C_MasterWrite( I2C_Addr_t slaveAddr, const I2C_Data_t *writeData )
{
    uint8_t     sreg;
    uint8_t     retryCount;
    I2C_Error_t rc;

    //  On the i2c bus, the following will take place (M=Master, S=Slave):
    //
    //  M->S:   Start
    //  M->S:   slave-addr w/ W
    //  M->S:   command
    //  M->S:   len
    //  M->S:   data (first byte should be address of master)
    //  M->S:   CRC
    //  M->S:   Stop

    // Wait for the i2c to become idle

    I2C_WaitForIdle();
    retryCount = 0;

    do
    {
        I2C_LOG2( "I2C_MasterWrite: Addr: 0x%02x Attempt: %d\n", slaveAddr, retryCount );

        sreg = SREG;    
        cli();
        {
            // Test again with interrupts disabled so we know it's still
            // really idle. Somebody could have addressed us right after
            // our check for IDLE.

            if ( gGlobals.m_state == I2C_STATE_IDLE )
            {
                gGlobals.m_state = I2C_STATE_MASTER_TX;

                gRetryRequired = 0;
                gErrorCode = I2C_ERROR_NONE;

                gGlobals.m_useCrc = (( slaveAddr & I2C_USE_CRC ) != 0 );

                gGlobals.m_deviceAddrRW = ( slaveAddr << 1 ) | TW_WRITE;
                gGlobals.m_writeData    = (char *)&writeData->m_command;
                gGlobals.m_writeDataLen = writeData->m_len + 2; // +1 for cmd, +1 for len
                gGlobals.m_writeDataIdx = 0;

                gGlobals.m_readData = NULL;

                gGlobals.m_crc = 0;

                // Kick things off by sending out a START

                I2C_SendStart();
                retryCount++;
            }
            else
            {
                // Some other type of transaction just started. Loop and
                // try again.

                gErrorCode = I2C_ERROR_ARBITRATION_LOST;
                gRetryRequired = 1;

                Log( "I2C_MasterWrite: not really idle\n" );
            }
        }
        SREG = sreg;

        // Wait for the transaction to complete. Otherwise we'll never know
        // if it actually happened or not. If we lose arbitration, then we
        // need to re-issue the write.

        I2C_WaitForIdle();
        rc = gErrorCode;

    } while ( gRetryRequired && ( retryCount < MAX_RETRIES ));

    if ( retryCount > 1 )
    {
        Log( "I2C_MasterWrite: took %d attempts\n", retryCount );
    }

    return rc;

} // I2C_MasterWrite

//***************************************************************************
/**
*   Process the bootloader commands.
*/

int ProcessCommand( I2C_Data_t *packet )
{
    I2C_LOG0( "Master ProcessCommand called\n" );

#if 0
    switch ( packet->m_command )
    {
        case 
    }
#endif

    return 0;

} // ProcessCommand
