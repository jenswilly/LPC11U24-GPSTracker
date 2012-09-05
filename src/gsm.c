/*
 * gsm.c
 *
 *  Created on: Sep 5, 2012
 *      Author: jenswilly
 */

#include <string.h>
#include "cmdparser.h"		// For OUTPUT_BUFFER_SIZE
#include "gsm.h"
#include "systick.h"
#include "lpc_swu.h"

#define GSMCAPTURE_CREG		(1<<0)
#define GSMCAPTURE_SMS		(1<<1)
#define GSMCAPTURE_OK		(1<<2)
#define GSMCAPTURE_ERROR	(1<<3)
volatile uint32_t gsm_capture;
volatile uint8_t gsm_responseOK;
volatile uint8_t gsm_responseERROR;

#define GSM_BUFFER_SIZE 100
char gsm_buffer[ GSM_BUFFER_SIZE ];
volatile uint16_t gsm_buffer_ptr;

/* Initializes internal state vars etc.
 */
void gms_init()
{
	// Zero pointers and capture states
	gsm_buffer_ptr = 0;
	gsm_capture = 0;

	gsm_responseOK = 0;
	gsm_responseERROR = 0;
}

/* Sends a command and waits the specified time for either OK or ERROR response.
 * The command should *not* include the trailing \r
 * Return values: OK=1, ERROR=0.
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
	swu_tx_str( command );
	swu_tx_chr( '\r' );

	// Wait (block) until we've received OK or ERROR or timeout occurs
	SysTick_Reset();
	while( !gsm_responseOK && !gsm_responseERROR && SysTick_Value < timeout )
		;

	// Return 1 if we got OK and 0 for timeout or ERROR
	return gsm_responseOK;
}

/* Soft UART RX handler
 */
void swu_rx_callback(void)
{
	// Append to buffer
	unsigned char data = swu_tx_chr();
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

static inline uint8_t check_tail( const char *command )
{
	uint8_t len = strlen( command );

	// If we haven't even received enough, return false
	if( gsm_buffer_ptr < len )
		return 0;

	return (strncasecmp( &gsm_buffer[ gsm_buffer_ptr-len ], command, len ) == 0 );

}

void gsm_do_AT( char *args, char *output )
{
	if( send_simplecommand( "AT" ))
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK\r\n" );
	else
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR\r\n" );
}
