/*
 * gsm.c
 *
 *  Created on: Sep 5, 2012
 *      Author: jenswilly
 */

#include <string.h>
#include <stdio.h>
#include "cmdparser.h"		// For OUTPUT_BUFFER_SIZE
#include "gsm.h"
#include "systick.h"
#include "lpc_swu.h"

#define GSMCAPTURE_CREG		(1<< 0)
#define GSMCAPTURE_SMS		(1<< 1)
#define GSMCAPTURE_OK		(1<< 2)
#define GSMCAPTURE_ERROR	(1<< 3)
volatile uint32_t gsm_capture;
volatile uint8_t gsm_responseOK;
volatile uint8_t gsm_responseERROR;

#define GSM_BUFFER_SIZE 100
char gsm_buffer[ GSM_BUFFER_SIZE ];
volatile uint16_t gsm_buffer_ptr;

char gsm_out_buffer[ 100 ];


/* Checks if the tail of the gsm_buffer matches the specified string.
 * Match is case-insensitive.
 * Return values: 1 = match; 0 = no match
 */
static inline uint8_t check_tail( const char *command )
{
	uint8_t len = strlen( command );

	// If we haven't even received enough, return false
	if( gsm_buffer_ptr < len )
		return 0;

	if( strncasecmp( &gsm_buffer[ gsm_buffer_ptr-len ], command, len ) == 0 )
		return 1;
	else
		return 0;
}

/* Initializes internal state vars etc.
 */
void gsm_init()
{
	// Zero pointers and capture states
	gsm_buffer_ptr = 0;
	gsm_capture = 0;

	gsm_capture = 0;
	gsm_responseOK = 0;
	gsm_responseERROR = 0;
}

/* Sends a command and waits the specified time for either OK or ERROR response.
 * The command should *not* include the trailing \r
 * Return values: 0 = OK, 1 = ERROR received, 2 = timeout
 * Timeout value is specified in current SysTick intervals.
 */
uint8_t send_simplecommand( char *command, uint32_t timeout )
{
	// Clear response flags
	gsm_responseOK = 0;
	gsm_responseERROR = 0;

	// We're waiting for OK or ERROR
	gsm_capture = GSMCAPTURE_OK | GSMCAPTURE_ERROR;

	// Send command and terminating \r
	swu_tx_str( (unsigned char*)command );
	swu_tx_chr( '\r' );

	// Wait (block) until we've received OK or ERROR or timeout occurs
	SysTick_Reset();
	while( !gsm_responseOK && !gsm_responseERROR && SysTick_Value < timeout )
		;

	// Return 1 if we got OK and 0 for timeout or ERROR
	if( gsm_responseERROR )
		return 1;
	if( SysTick_Value >= timeout )
			return 2;

	return 0;
//	return gsm_responseOK;
}

/* Soft UART RX handler.
 * Each char is added to the gsm_buffer. If the char is \n we assume that a complete line has been received and it
 * will be parsed.
 * Parsing is done by looking for specific strings (only those specified in gsm_capture are checked for) and setting the corresponding
 * flags or values.
 * The buffer is reset when the line is parsed.
 */
void swu_rx_callback(void)
{
	// Append to buffer
	unsigned char data = swu_rx_chr();
	gsm_buffer[ gsm_buffer_ptr++ ] = data;

	// Check for EOL
	if( data == '\n' )
	{
		// Yes, EOL. See what we're currently looking for at update vars appropriately

		// OK\r\n
		if( gsm_capture & GSMCAPTURE_OK )
			gsm_responseOK = check_tail( "OK\r\n" );

		// ERROR\r\n
		if( gsm_capture & GSMCAPTURE_ERROR )
			gsm_responseERROR = check_tail( "ERROR\r\n" );

		// The line has been handled: reset the counter
		gsm_buffer_ptr = 0;
	}


	// Wrap around if overflow
	gsm_buffer_ptr %= GSM_BUFFER_SIZE;
}

void gsm_do_AT( char *args, char *output )
{
	uint8_t result = send_simplecommand( "AT", 20 );

	if( result == 0)	// AT, 2 secs timeout
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK\r\n" );
	else if( result == 1 )
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR\r\n" );
	else if( result == 2 )
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - timeout\r\n" );
}

void gsm_do_pin( char *args, char *output )
{
	// Append specified pin
	strcpy( gsm_out_buffer, "AT+CPIN=0000");
	memcpy( gsm_out_buffer+8, args, 4 );

	uint8_t result = send_simplecommand( gsm_out_buffer, 50 );
	if( result == 0)	// AT, 2 secs timeout
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK\r\n" );
	else if( result == 1 )
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR\r\n" );
	else if( result == 2 )
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - timeout\r\n" );
}

void gsm_do_raw( char *args, char *output )
{
	// Send args directly to GSM
	uint8_t result = send_simplecommand( args, 50 );
	if( result == 0)	// AT, 2 secs timeout
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK\r\n" );
	else if( result == 1 )
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR\r\n" );
	else if( result == 2 )
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - timeout\r\n" );
}
