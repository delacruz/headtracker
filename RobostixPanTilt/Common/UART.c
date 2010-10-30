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
*   @file   UART.cpp
*
*   @brief  Class for abstracting an interrupt driven UART
*
****************************************************************************/

// ---- Include Files -------------------------------------------------------

#include "Config.h"
#include "CBUF.h"
#include "UART.h"

#include <avr/io.h>
#if defined( __AVR_LIBC_VERSION__ )
#   include <avr/interrupt.h>
#else
#include <avr/signal.h>
#endif

// ---- Public Variables ----------------------------------------------------
// ---- Private Constants and Types -----------------------------------------

// ---- Private Variables ---------------------------------------------------

#if CFG_USE_UART0

#if ( CFG_UART0_RX_BUFFER_SIZE > 0 )

volatile UART0_RxBuffer_t gUart0RxBuf;

#else

void (*gUart0RxHandler)( uint8_t ch ) = 0;

#endif

#if ( CFG_UART0_TX_BUFFER_SIZE > 0 )
volatile UART0_TxBuffer_t gUart0TxBuf;
#endif

#endif  // CFG_USE_UART0

//---------------------------------------------------------------------------

#if CFG_USE_UART1

#if ( CFG_UART1_RX_BUFFER_SIZE > 0 )

volatile UART1_RxBuffer_t gUart1RxBuf;

#else

void (*gUart1RxHandler)( uint8_t ch ) = 0;

#endif

#if ( CFG_UART1_TX_BUFFER_SIZE > 0 )
volatile UART1_TxBuffer_t gUart1TxBuf;
#endif

#endif  // CFG_USE_UART1

// ---- Private Function Prototypes -----------------------------------------
// ---- Functions -----------------------------------------------------------

/*
 * We assume that all of the appropriate registers are initialized
 * in the InitHardware function.
 */

//***************************************************************************
/**
*   Interrupt handler for Uart Rx Complete
*/

#if CFG_USE_UART0

SIGNAL( SIG_USART0_RECV )
{
    uint8_t ch = UDR0;   // Read the character from the UART

#if ( CFG_UART0_RX_BUFFER_SIZE > 0 )

    if ( !CBUF_IsFull( gUart0RxBuf ))
    {
        CBUF_Push( gUart0RxBuf, ch );
    }

#else
    if ( gUart0RxHandler != 0 )
    {
        gUart0RxHandler( ch );
    }
#endif

} // SIG_USART0_RECV

#endif  // CFG_USE_UART0

//---------------------------------------------------------------------------

#if CFG_USE_UART1

SIGNAL( SIG_USART1_RECV )
{
    uint8_t ch = UDR1;   // Read the character from the UART

#if ( CFG_UART1_RX_BUFFER_SIZE > 0 )

    if ( !CBUF_IsFull( gUart1RxBuf ))
    {
        CBUF_Push( gUart1RxBuf, ch );
    }

#else
    if ( gUart1RxHandler != 0 )
    {
        gUart1RxHandler( ch );
    }
#endif

} // SIG_USART1_RECV

#endif  // CFG_USE_UART1

//***************************************************************************
/**
*   Interrupt handler for Uart Data Register Empty
*/

#if ( CFG_USE_UART0 && ( CFG_UART0_TX_BUFFER_SIZE > 0 ))

SIGNAL( SIG_USART0_DATA )
{
    if ( CBUF_IsEmpty( gUart0TxBuf ))
    {
        // Nothing left to transmit, disable the transmit interrupt

        UCSR0B &= ~( 1 << UDRIE0 );
    }
    else
    {
        // Otherwise, write the next character from the TX Buffer

        UDR0 = CBUF_Pop( gUart0TxBuf );
    }

}  // SIG_USART0_DATA

#endif  // ( CFG_USE_UART0 && ( CFG_UART0_TX_BUFFER_SIZE > 0 ))

//---------------------------------------------------------------------------

#if ( CFG_USE_UART1 && ( CFG_UART1_TX_BUFFER_SIZE > 0 ))

SIGNAL( SIG_USART1_DATA )
{
    if ( CBUF_IsEmpty( gUart1TxBuf ))
    {
        // Nothing left to transmit, disable the transmit interrupt

        UCSR1B &= ~( 1 << UDRIE1 );
    }
    else
    {
        // Otherwise, write the next character from the TX Buffer

        UDR1 = CBUF_Pop( gUart1TxBuf );
    }

}  // SIG_USART1_DATA

#endif  // ( CFG_USE_UART1 && ( CFG_UART1_TX_BUFFER_SIZE > 0 ))

//***************************************************************************
/**
*   Extracts a character from the receive buffer. This function will block
*   until a character is available.
*
*   This function returns an int (rather than a char) to be compatible 
*   with fdevopen()
*/

#if CFG_USE_UART0
#if ( CFG_UART0_RX_BUFFER_SIZE > 0 )

#if defined( __AVR_LIBC_VERSION__ )
int UART0_GetCharStdio( FILE *fs )
{
    return UART0_GetChar();
}
#endif

int UART0_GetChar( void )
{
    int ch;

    while ( CBUF_IsEmpty( gUart0RxBuf ))
    {
        ;
    }

    ch = CBUF_Pop( gUart0RxBuf );

#if CFG_UART0_CR_TO_LF

    // The following allows this to be used with fgets

    if ( ch == '\r' )
    {
        ch = '\n';
    }
#endif
#if CFG_UART0_ECHO_INPUT
    UART0_PutChar( ch );
#endif

    return ch;

} // UART_GetChar
#endif  // CFG_UART0_RX_BUFFER_SIZE > 0
#endif  // CFG_USE_UART0

//---------------------------------------------------------------------------

#if CFG_USE_UART1
#if ( CFG_UART1_RX_BUFFER_SIZE > 0 )

#if defined( __AVR_LIBC_VERSION__ )
int UART1_GetCharStdio( FILE *fs )
{
    return UART1_GetChar();
}
#endif

int UART1_GetChar( void )
{
    int ch;

    while ( CBUF_IsEmpty( gUart1RxBuf ))
    {
        ;
    }

    ch = CBUF_Pop( gUart1RxBuf );

#if CFG_UART1_CR_TO_LF

    // The following allows this to be used with fgets

    if ( ch == '\r' )
    {
        ch = '\n';
    }
#endif
#if CFG_UART1_ECHO_INPUT
    UART1_PutChar( ch );
#endif

    return ch;

} // UART_GetChar
#endif  // CFG_UART1_RX_BUFFER_SIZE > 0
#endif  // CFG_USE_UART1

//***************************************************************************
/**
*   Write's a single character to the UART. By returning an int, this
*   function is compatible with fdevopen()
*/

#if CFG_USE_UART0

#if defined( __AVR_LIBC_VERSION__ )
int UART0_PutCharStdio( char ch, FILE *fs )
{
    return UART0_PutChar( ch );
}
#endif

int UART0_PutChar( char ch )
{
#if CFG_UART0_LF_TO_CRLF
    if ( ch == '\n'  )
    {
        UART0_PutChar( '\r' );
    }
#endif

#if ( CFG_UART0_TX_BUFFER_SIZE > 0 )

    while ( CBUF_IsFull( gUart0TxBuf ))
    {
        ;
    }

    CBUF_Push( gUart0TxBuf, ch );

    // Enable the transmit interrupt now that there's a character in the
    // buffer.

    UCSR0B |= ( 1 << UDRIE0 );

#else

    // Wait for empty transmit buffer

    while (( UCSR0A & ( 1 << UDRE0 )) == 0 )
    {
        ;
    }

    // Send the character

    UDR0 = ch;
#endif

    return 0;

} // UART0_PutChar

#endif  // CFG_USE_UART0

//---------------------------------------------------------------------------

#if CFG_USE_UART1

#if defined( __AVR_LIBC_VERSION__ )
int UART1_PutCharStdio( char ch, FILE *fs )
{
    return UART1_PutChar( ch );
}
#endif

int UART1_PutChar( char ch )
{
#if CFG_UART1_LF_TO_CRLF
    if ( ch == '\n'  )
    {
        UART1_PutChar( '\r' );
    }
#endif

#if ( CFG_UART1_TX_BUFFER_SIZE > 0 )

    while ( CBUF_IsFull( gUart1TxBuf ))
    {
        ;
    }

    CBUF_Push( gUart1TxBuf, ch );

    // Enable the transmit interrupt now that there's a character in the
    // buffer.

    UCSR1B |= ( 1 << UDRIE1 );

#else

    // Wait for empty transmit buffer

    while (( UCSR1A & ( 1 << UDRE1 )) == 0 )
    {
        ;
    }

    // Send the character

    UDR1 = ch;
#endif

    return 0;

} // UART1_PutChar

#endif  // CFG_USE_UART1

//***************************************************************************
/**
*   Write a string to the UART.
*/

#if CFG_USE_UART0

void UART0_PutStr( const char *str )
{
    while ( *str != '\0' )
    {
        if ( *str == '\n' )
        {
            UART0_PutChar( '\r' );
        }
        UART0_PutChar( *str );

        str++;
    }

} // UART0_PutStr

#endif  // CFG_USE_UART0

//---------------------------------------------------------------------------

#if CFG_USE_UART1

void UART1_PutStr( const char *str )
{
    while ( *str != '\0' )
    {
        if ( *str == '\n' )
        {
            UART1_PutChar( '\r' );
        }
        UART1_PutChar( *str );

        str++;
    }

} // UART1_PutStr

#endif  // CFG_USE_UART1

//***************************************************************************
/**
*   Sets the function that will be called to process a received character
*/

#if CFG_USE_UART0
#if ( CFG_UART0_RX_BUFFER_SIZE == 0 )
void UART0_SetRxHandler( void (*rxHandler)( uint8_t ch ))
{
    gUart0RxHandler = rxHandler;

} // UART0_SetRxHandler
#endif
#endif

//---------------------------------------------------------------------------

#if CFG_USE_UART1
#if ( CFG_UART1_RX_BUFFER_SIZE == 0 )
void UART1_SetRxHandler( void (*rxHandler)( uint8_t ch ))
{
    gUart1RxHandler = rxHandler;

} // UART1_SetRxHandler
#endif
#endif

//***************************************************************************
/**
*   Write data to the UART. This function may block if insufficient space
*   is available in the transmit buffer.
*/

#if CFG_USE_UART0

void UART0_Write( const void *data, uint8_t len )
{
    const uint8_t *p = (const uint8_t *)data;

    while ( len > 0 )
    {
        UART0_PutChar( *p++ );
        len--;
    }

} // UART0_Write

#endif

//---------------------------------------------------------------------------

#if CFG_USE_UART1

void UART1_Write( const void *data, uint8_t len )
{
    const uint8_t *p = (const uint8_t *)data;

    while ( len > 0 )
    {
        UART1_PutChar( *p++ );
        len--;
    }

} // UART1_Write

#endif

