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
#include "gps.h"

/* Buffers.
 * We use two buffers: one for receiving data and one for parsing NMEA messages.
 * Two pointers, receive_buffer and nmea_buffer point to the current receive and NMEA parse buffer.
 * When a complete line has been received, the two buffers are switched so reception continues in the second buffer while the first buffer is being parsed.
 */
#define GPS_BUFFER_SIZE 100
char gps_buffer1[ GPS_BUFFER_SIZE ];
//char gps_buffer2[ GPS_BUFFER_SIZE ];
volatile uint8_t is_parsing;				// Semaphore: do not accept GPS data when parsing
char *rx_buffer;
char *nmea_buffer;
volatile uint16_t rx_buffer_ptr;
uint8_t gps_rawecho;
GPSStatus gps_status;
volatile uint8_t all_GSV_messages_received;	// Parsing state var: has all GSV messages been recieved? (Since one GSV message contains max. four sats)
volatile uint8_t GSV_messages;				// Parsing state var: number of GSV messages received.
uint8_t	segments[ 50 ];
volatile uint8_t segments_index;

/* CT32B0 interrupt handler.
 * Calls soft UART functions.
 */
void TIMER32_0_IRQHandler( void )
{
	swu_isr_rx( LPC_CT32B0 );
	swu_isr_tx( LPC_CT32B0 );
}

/* Receive GPS NMEA messages on USART0.
 */
void UART_IRQHandler(void)
{
	unsigned char data = LPC_USART->RBR;
//	char *tmp_ptr;

	// Are we are currently parsing?
	if( is_parsing == 1 )
		// Yes we are: don't accept data at the moment
		return;

	switch( data )
	{
		case '$':
			// Note: the '$' is *not* added to the string buffer
			rx_buffer_ptr = 0;
			segments_index = 0;
			break;

		case '\r':
			// NOTE: The \r is *not* added to the buffer. The following \n will be added but this will be discarded when the next $ is received

			// Parse only GPRMC and GPGSV messages
			if( strncmp( "GPRMC", rx_buffer, 5 ) == 0 )
			{
				// Switch buffers
//				tmp_ptr = nmea_buffer;
//				nmea_buffer = rx_buffer;
//				rx_buffer = tmp_ptr;

				// Parse MNEA message
				is_parsing = 1;
				state = StateGPSParseRMC;
			}
			else if( strncmp( "GPGSV", rx_buffer, 5 ) == 0 )
			{
				// Switch buffers
//				tmp_ptr = nmea_buffer;
//				nmea_buffer = rx_buffer;
//				rx_buffer = tmp_ptr;

				// Parse MNEA message
				is_parsing = 1;
				state = StateGPSParseGSV;
			}

			// Otherwise it's not GPRMC or GPGSV so we'll ignore the entire message Ð i.e. keep bufering until we get the next $ character
			break;

		default:
			// Buffering
			if( data == ',' || data == '*' )
				segments[ segments_index++ ] = rx_buffer_ptr;

			rx_buffer[ rx_buffer_ptr++ ] = data;
			break;
	}
}

/* Receive GPS NMEA messages on soft UART.
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

/* Initializes structures and variables
 */
void gps_init( void )
{
	// Initialize buffer pointer and rawecho state
	rx_buffer_ptr = 0;
	gps_rawecho = 0;
	rx_buffer = gps_buffer1;
	nmea_buffer = gps_buffer1;

	// Initialize GPS status struct
	memset( gps_status.lat, sizeof( gps_status.lat ), 0 );
	memset( gps_status.lng, sizeof( gps_status.lng ), 0 );
	memset( gps_status.timestamp, sizeof( gps_status.timestamp ), 0 );
	gps_status.sats_in_view = 0;
	gps_status.sats_tracking = 0;
	gps_status.status = 0;

	// Init parsing state vars
	all_GSV_messages_received = 1;
	GSV_messages = 0;
	segments_index = 0;

	// Zero buffers
    memset( gps_buffer1, GPS_BUFFER_SIZE, 0 );
//    memset( gps_buffer2, GPS_BUFFER_SIZE, 0 );
}

/**********
 * NMEA parsing functions
 */
void parse_RMC()
{
	uint8_t i;
	/*	indices:
		0	UTC time in HHMMSS.SSSS
		1	status; V=not valid, A=valid fix
		2	latitude in DDMM.MMMM
		3	latitude N/S
		4	longitude in DDDMM.MMMM
		5	longitude E/W
		6	speed in knots
		7	course
		8	UTC date DDMMYY
	    9	(not used)
	   10	(not used)
	   11	mode indicator: ÔNÕ = Data not valid, ÔAÕ = Autonomous mode, ÔDÕ = Differential mode, ÔEÕ = Estimated (dead reckoning) mode, ÔMÕ = Manual input mode, ÔSÕ = Simulator mode
	 */

	// Time: copy 10 characters from segments[0]+1 (+1 to skip the comma)
	char *data_ptr = gps_status.timestamp;
	for( i=0; i < 10; i++ )
	{
		data_ptr[i] = nmea_buffer[ segments[0]+1 + i ];
	}

	// Latitude: copy 9 characters from segment 2
	data_ptr = gps_status.lat;
	for( i=0; i< 9; i++ )
		data_ptr[i] = nmea_buffer[ segments[2]+1 + i ];

	// Lat N/S: copy 1 character from segment 3
	data_ptr[9] = nmea_buffer[ segments[3] + 1 ];

	// Longitude: copy 10 characters from segment 4
	data_ptr = gps_status.lng;
	for( i=0; i< 10; i++ )
		data_ptr[i] = nmea_buffer[ segments[4]+1 + i ];

	// Lng E/W: copy 1 character from segment 5
	data_ptr[10] = nmea_buffer[ segments[5] + 1 ];

	// Status
	gps_status.status = (nmea_buffer[ segments[1]+1 ] == 'A');

	// Parsing done: clear semaphore
	is_parsing = 0;
}

void parse_GSV()
{
	uint8_t i;
	/*	indices:
		0	number of GSV messages (1-3)
		1	number of current GSV message
		2	total sats in view
		3	sat number
		4	sat elevation
		5	sat azimuth
		6	sat SNR; empty if not tracking
	?	7	sat number
	?	8	sat elevation
	?	9	sat azimuth
	?  10	sat SNR; empty if not tracking
	?  11	sat number
	?  12	sat elevation
	?  13	sat azimuth
	?  14	sat SNR; empty if not tracking
	?  15	sat number
	?  16	sat elevation
	?  17	sat azimuth
	?  18	sat SNR; empty if not tracking
	 */

	GSV_messages = nmea_buffer[ segments[0]+1 ] - '0';
	int GSV_message_index = nmea_buffer[ segments[1]+1 ] - '0';

	// if first GSV message, reset sats_in_view and _tracking
	if( GSV_message_index == 1 )
	{
		gps_status.sats_in_view = 0;
		gps_status.sats_tracking = 0;
	}

	// total number of sats in view
	// one or two digits?
	if( segments[3]-segments[2] == 2 )
		// just one
		gps_status.sats_in_view = nmea_buffer[ segments[2]+1 ] - '0';
	else
		// two digits
		gps_status.sats_in_view = (nmea_buffer[ segments[2]+1 ] - '0') * 10 + nmea_buffer[ segments[2]+2 ] - '0';

	// how many satellite to except in this message
	int expecting_now = gps_status.sats_in_view - (GSV_message_index-1) * 4;
	if( expecting_now > 4 )
		expecting_now = 4;

	// get sat infos
	for( i=0; i<expecting_now; i++ )
	{
		// sat no.
		// ...

		// elevation
		// ...

		// azimuth
		// ...

		// tracking?
		if( segments[ i*4 + 7 ] - segments[ i*4 + 6 ] > 1 )
			gps_status.sats_tracking++;
	}

	// have we received all GSV messages?
	all_GSV_messages_received = (GSV_message_index == GSV_messages);

	// Parsing done: clear semaphore
	is_parsing = 0;
}

/**********
 * Command functions
 */

void gps_do_status( char *args, char *output )
{
	// Print current GPS status
	sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - GPS status: %d, sats: %d/%d, time: %s, lat: %s, lng: %s\r\n", gps_status.status,
			gps_status.sats_tracking,
			gps_status.sats_in_view,
			gps_status.timestamp,
			gps_status.lat,
			gps_status.lng );
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


