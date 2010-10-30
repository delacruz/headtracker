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
#include <compat/twi.h>

#include "i2c-slave.h"
#include "Crc8.h"
#include "Log.h"
#include "Config.h"

/* ---- Public Variables -------------------------------------------------- */
/* ---- Private Constants and Types --------------------------------------- */

/* ---- Private Variables ------------------------------------------------- */

/* ---- Private Function Prototypes --------------------------------------- */

/* ---- Functions --------------------------------------------------------- */

/**
 * @addtogroup I2C
 * @{
 */

//***************************************************************************
/**
*   I2C Interrupt service routine
*
*/

uint8_t I2C_SlaveHandler( I2C_Globals_t *globals )
{
    // The TWI handling is basically a state machine driven by the status
    // bits presented by the hardware.

	uint8_t status = TW_STATUS;
    uint8_t twcr   = TWCR;

    // Using if instead of switch saves over 100 bytes from a simple and 12
    // bytes smaller than a switch when status >> 3 is used (and we get
    // a jumptable)

    // We use do { } while ( 0 ) so that we can make break and "fall through" possible.

    do
    {
        if ( status == TW_BUS_ERROR )  // 0x00 - Bus error due to illegal start otstop condition
        {
            I2C_LOG0( "BUS-ERROR\n" );
            break;
        }

        // ----- Slave Receiver States -----

        if (( status == TW_SR_SLA_ACK )             // 0x60 - SLA+W received ACK sent
        ||  ( status == TW_SR_ARB_LOST_SLA_ACK )    // 0x68 - SLA+W received while trying to send SLA+R
        ||  ( status == TW_SR_GCALL_ACK )           // 0x70 - GCA+W received, ACK sent
        ||  ( status == TW_SR_ARB_LOST_GCALL_ACK )) // 0x78 - CGA+W received, ACK sent (while trying to send SLA+R/W)
        {
#if I2C_LOG_ENABLED
            if ( status == TW_SR_SLA_ACK )
            {
                I2C_LOG0( "SR-SLA-ACK\n" );
            }
            else
            if ( status == TW_SR_ARB_LOST_SLA_ACK )
            {
                I2C_LOG0( "SR-ARB-LOST-SLA-ACKK\n" );
            }
            else
            if ( status == TW_SR_GCALL_ACK )
            {
                I2C_LOG0( "SR-GCALL-ACK\n" );
            }
            else
            {
                I2C_LOG0( "SR-ARB-LOST-GCALL-ACK\n" );
            }
#endif

            if (( status == TW_SR_GCALL_ACK ) || ( status == TW_SR_ARB_LOST_GCALL_ACK ))
            {
                globals->m_deviceAddrRW = 0;
            }
            else
            {
                globals->m_deviceAddrRW = TWAR & ~TW_READ;
            }

            // We've been addressed to receive data, and we've acknowledged the address.
            // The next interrupt will be the first byte of data (TW_SR_DATA_ACK)

            globals->m_state = I2C_STATE_SLAVE_RX;

            memset( &globals->m_slavePacket, 0, sizeof( globals->m_slavePacket ));

            globals->m_readData = &globals->m_slavePacket.m_data[ 0 ];
            globals->m_readDataLen = 0;
            globals->m_readDataValid = 0;

            I2C_CRC( globals->m_crc = Crc8( 0, globals->m_deviceAddrRW ); )
            break;
        }

        if (( status == TW_SR_DATA_ACK )        // 0x80 - Data received for SLA+W;,ACK sent
        ||  ( status == TW_SR_GCALL_DATA_ACK )) // 0x90 - Data received for GCA+W, ACK sent
        {
            uint8_t data = TWDR;

            if ( status == TW_SR_DATA_ACK )
            {
                I2C_LOG1( "SR-DATA-ACK 0x%02x\n", data );
            }
            else
            {
                I2C_LOG1( "SR-GCALL-DATA-ACK 0x%02x\n", data );
            }

            // Write requests with 32 bytes of data and a CRC cause 33 bytes
            // of data to be received. We won't store the 33rd byte, but
            // we will add it into the CRC.

            if ( globals->m_readDataLen < sizeof( globals->m_slavePacket.m_data ))
            {
                globals->m_readData[ globals->m_readDataLen ] = data;
            }
            else
            {
                // We can't receive any more data. We ack the 33rd byte
                // because it's probably the CRC. We'll NAK everything beyond
                // that. Rmember that the NAK doesn't take effect until 
                // after receiving the next byte.

                if ( globals->m_readDataLen > sizeof( globals->m_slavePacket.m_data ))
                {
                    I2C_LOG1( "readDataLen too big: %d, clearing TWEA\n", globals->m_readDataLen );

                    twcr &= ~( 1 << TWEA );
                    globals->m_state = I2C_STATE_IDLE;
                }
            }
            globals->m_readDataLen++;

            I2C_CRC( globals->m_crc = Crc8( globals->m_crc, data ); )
            break;
        }

        if (( status == TW_SR_DATA_NACK )           // 0x88 - Data received, NACK sent
        ||  ( status == TW_SR_GCALL_DATA_NACK ))    // 0x98 - Data received, NACK sent
        {
            if ( status == TW_SR_DATA_NACK )
            {
                I2C_LOG0( "SR-DATA-NACK\n" );
            }
            else
            {
                I2C_LOG0( "SR-GCALL-DATA-NACK\n" );
            }

            // The hardware now switches to the not addressed slave mode and
            // we'll once again recognize our own address

            twcr |= ( 1 << TWEA );
            break;
        }

        if ( status == TW_SR_STOP ) // 0xA0 - STOP or repeated START condition has been received while still addressed as Slave
        {
            I2C_LOG0( "SR-STOP\n" );

#if defined( CFG_I2C_LED_PORT )
            CFG_I2C_LED_PORT ^= CFG_I2C_LED_MASK;
#endif

            if ( globals->m_state == I2C_STATE_SLAVE_RX )
            {
                // We were addressed as a slave receiver. The SR-STOP indicates
                // that all of the data that will be sent to us has been,
                // so we call the callback routine.
                //
                // The callback should return the length of data that we have
                // to send back to the master (and should return zero if the
                // transaction is intended to be strictly master to slave).

                if ( globals->m_processCommand != NULL )
                {
                    globals->m_slavePacket.m_len = globals->m_readDataLen;

                    I2C_LOG0( "Calling ProcessCommand\n" );
                    globals->m_slavePacket.m_len = globals->m_processCommand( &globals->m_slavePacket );
                    I2C_LOG1( "ProcessCommand returned len of %d\n", globals->m_slavePacket.m_len );
                }
                else
                {
                    globals->m_slavePacket.m_len = 0;
                }

                if ( globals->m_slavePacket.m_len  > 0 )
                {
                    // Set things up so that when we're addressed as a slave
                    // transmitter that we'll send back the right data.

                    globals->m_writeData = &globals->m_slavePacket.m_data[ 0 ];
                    globals->m_writeDataLen = globals->m_slavePacket.m_len;
                }
                else
                {
                    globals->m_writeData = NULL;
                    globals->m_writeDataLen = 0;
                    globals->m_state = I2C_STATE_IDLE;
                }
            }

            // We can't tell if this was a STOP or a REPEATED-START. If it was
            // a STOP, then we're done. If it was a repeated start we'll get
            // readdressed.

            globals->m_state = I2C_STATE_IDLE;
            break;
        }

        // ----- Slave Transmitter States -----

        if (( status == TW_ST_ARB_LOST_SLA_ACK )    // 0xB0 - Arbitration lost as master trying to send SLA+W. Rcvd SLA+R
        ||  ( status == TW_ST_SLA_ACK ))            // 0xA8 - Own SLA+R has been received; ACK has been returned
        {
            if ( status == TW_ST_SLA_ACK )
            {
                I2C_LOG0( "ST-SLA-ACK\n" );
            }
            else
            {
                I2C_LOG0( "ST-ARB-LOST-SLA-ACK\n" );
            }

            if ( globals->m_state == I2C_STATE_IDLE )
            {
                globals->m_state = I2C_STATE_SLAVE_TX;
            }

            globals->m_writeDataIdx = 0;
            I2C_CRC( globals->m_crc = Crc8( globals->m_crc, TWAR | TW_READ ); )

            status = TW_ST_DATA_ACK;
        }
        if ( status == TW_ST_DATA_ACK )    // 0xB8 - Data byte in TWDR has been transmitted; ACK has been received
        {
            uint8_t data;

            // We're in the middle of sending data, send the next byte.

            if ( globals->m_writeDataIdx > globals->m_writeDataLen )
            {
                I2C_LOG0( "ST-DATA-ACK: Out of Data, sending 0x00\n" );

                data = 0;
            }
#if CFG_I2C_USE_CRC
            else
            if ( globals->m_writeDataIdx == globals->m_writeDataLen )
            {
                // We've sent all of the data, send the CRC. This is the last
                // data that we'll send.

                data = globals->m_crc;

                I2C_LOG1( "ST-DATA-ACK: Sending CRC 0x%02x\n", data );

                twcr &= ~( 1 << TWEA );
            }
#endif
            else
            {
                data = globals->m_writeData[ globals->m_writeDataIdx++ ];

                I2C_LOG2( "ST-DATA-ACK: Sending 0x%02x twcr:0x%02x\n", data, twcr );

#if CFG_I2C_USE_CRC
                I2C_CRC( globals->m_crc = Crc8( globals->m_crc, data ); )
#else
                if ( globals->m_writeDataIdx == globals->m_writeDataLen )
                {
                    // This is the last byte, Send  
                    // data that we'll send.

                    data = globals->m_crc;

                    I2C_LOG1( "ST-DATA-ACK: Sending CRC 0x%02x\n", data );

                    twcr &= ~( 1 << TWEA );
                }
#endif
            }
            TWDR = data;
            break;
        }

        if ( status == TW_ST_DATA_NACK )   // 0xC0 - Data byte in TWDR has been transmitted; NACK has been received.
        {
            I2C_LOG0( "ST-DATA-NACK\n" );
            globals->m_state = I2C_STATE_IDLE;
            twcr |= ( 1 << TWEA );
            break;
        }

        if ( status == TW_ST_LAST_DATA )   // 0xC8 - Last data byte in TWDR has been transmitted (TWEA = 0); ACK has been received
        {
            I2C_LOG0( "ST-LAST-DATA\n" );
            globals->m_state = I2C_STATE_IDLE;
            twcr |= ( 1 << TWEA );
            break;     
        }

        // We don't recognize the status - must be a master status

        return 0;

    } while( 0 );

    // Setup the updated TWCR value, but DON'T clear the TWINT bit
    // Ironically, you clear the TWINT bit by writing a 1 to it.

    TWCR = twcr & ~( 1 << TWINT );

    //I2C_LOG1( "TWCR:0x%02x\n", TWCR );

    return 1;

} // I2C_SlaveHandler


