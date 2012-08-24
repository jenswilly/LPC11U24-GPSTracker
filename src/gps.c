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

#define GPS_BUFFER_SIZE 100
char gps_buffer[ GPS_BUFFER_SIZE ];
volatile uint16_t gps_buffer_ptr;
volatile uint8_t gps_rawecho;

/* Soft UART data received callback.
 */
void swu_rx_callback(void)
{
	unsigned char data = swu_rx_chr();

	// We'll ignore any \r
	if( data == '\r' )
		return;

	// If it's \n, we'll terminate the string and change state to GPS line received.
	if( data == '\n' )
	{
		gps_buffer[ gps_buffer_ptr ] = 0;
		state = StateGPSLineReceived;

		// And reset the buffer pointer
		gps_buffer_ptr = 0;

		return;
	}

	// Otherwise, we got a normal character: append it to the buffer
	gps_buffer[ gps_buffer_ptr++ ] = data;

	// Check overflow
	if( gps_buffer_ptr >= GPS_BUFFER_SIZE )
		gps_buffer_ptr = 0;
}

void gps_init( void )
{
	// Initialize soft UART using CT32B0 timer
	swu_init( LPC_CT32B0 );

	// Initialize buffer pointer and rawecho state
	gps_buffer_ptr = 0;
	gps_rawecho = 0;
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
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - GPS echo is OFF" );
	else
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - GPS echo is ON" );
}


