/*
 * gps.c
 *
 *  Created on: Aug 24, 2012
 *      Author: jenswilly
 *
 *  This file contains support and command functions dealing with the GPS module.
 *  Communication is by way of the soft UART interface on P0.17 and P0.11.
 */

#include <string.h>
#include <stdio.h>
#include "cmdparser.h"		// For OUTPUT_BUFFER_SIZE
#include "lpc_swu.h"
#include "main.h"
#include "usb_cdc.h"

#define GPS_BUFFER_SIZE 500
char gps_buffer[ GPS_BUFFER_SIZE ];
volatile uint16_t gps_buffer_ptr;
volatile uint8_t gps_rawecho;

/* CT32B0 interrupt handler.
 * Calls soft UART functions.
 */
void TIMER32_0_IRQHandler( void )
{
	swu_isr_rx( LPC_CT32B0 );
	swu_isr_tx( LPC_CT32B0 );
}

void UART_IRQHandler(void)
{
	unsigned char data = LPC_USART->RBR;

	// If it's \n, we'll terminate the string and change state to GPS line received.
	if( data == '\n' )
	{
		// Terminate string and copy to output buffer
		if( gps_rawecho ==  1)
		{
			gps_buffer[ gps_buffer_ptr++ ] = data;
			gps_buffer[ gps_buffer_ptr++ ] = '\0';

			memcpy( response_buffer, gps_buffer, gps_buffer_ptr );
		}

		// And reset the buffer pointer
		gps_buffer_ptr = 0;

		// Set state
		state = StateGPSLineReceived;
		return;
	}

	// Otherwise, we got a normal character: append it to the buffer
	gps_buffer[ gps_buffer_ptr++ ] = data;

	// Check overflow
	if( gps_buffer_ptr >= GPS_BUFFER_SIZE )
		gps_buffer_ptr = 0;
}

/* Soft UART data received callback.
 */
/*
void swu_rx_callback(void)
{
	unsigned char data = swu_rx_chr();

	// We'll ignore any \r
//	if( data == '\r' )
//		return;

	// If it's \n, we'll terminate the string and change state to GPS line received.
	if( data == '\n' )
	{
		// Terminate string and copy to output buffer
		if( gps_rawecho ==  1)
		{
			gps_buffer[ gps_buffer_ptr++ ] = data;
			gps_buffer[ gps_buffer_ptr++ ] = '\0';

			memcpy( response_buffer, gps_buffer, gps_buffer_ptr );
		}

		// And reset the buffer pointer
		gps_buffer_ptr = 0;

		// Set state
		state = StateGPSLineReceived;
		return;
	}

	// Otherwise, we got a normal character: append it to the buffer
	gps_buffer[ gps_buffer_ptr++ ] = data;

	// Check overflow
	if( gps_buffer_ptr >= GPS_BUFFER_SIZE )
		gps_buffer_ptr = 0;
}
*/
void gps_init( void )
{
	// Initialize buffer pointer and rawecho state
	gps_buffer_ptr = 0;
	gps_rawecho = 0;

	// Zero buffer
    memset( gps_buffer, GPS_BUFFER_SIZE, 0 );
}

void gps_do_init( char *args, char *output )
{
	// Init GPS
	gps_init();

	sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - GPS initialized\r\n" );
}

void gps_do_rawecho( char *args, char *output )
{
	// Arguments?
	if( args != 0 )
	{
		// Arguments: set echo ON (1) or OFF (0)
		if( strncmp( "1", args, 1 ) == 0 || strncasecmp( "on", args, 2 ) == 0 )
			gps_rawecho = 1;
		else
			gps_rawecho = 0;
	}

	// And print current value. We'll do that whether we got arguments or not
	if( gps_rawecho == 0 )
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - GPS echo is OFF\r\n" );
	else
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - GPS echo is ON\r\n" );
}


