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

#define STANDARD_TIMEOUT 20	// 2 secs std. timeout value

#define GSMCAPTURE_CREG			(1<< 0)
#define GSMCAPTURE_SMS			(1<< 1)
#define GSMCAPTURE_OK			(1<< 2)
#define GSMCAPTURE_ERROR		(1<< 3)
#define GSMCAPTURE_SMSPROMPT	(1<< 4)
volatile uint32_t gsm_capture_mask;
volatile uint32_t gsm_capture_state;

volatile char gsm_responseCREG;	// The value of the last AT+CREG? request. '0' = not searching, '1' = connected, '2' = searching, '3' = denied (possibly wrong network)

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

/* Checks if the start of the gsm_buffer matches the specified string.
 * Match is case-insensitive.
 * Return values: 1 = match; 0 = no match
 */
static inline uint8_t check_head( const char *command )
{
	uint8_t len = strlen( command );

	// If we haven't even received enough, return false
	if( gsm_buffer_ptr < len )
		return 0;

	if( strncasecmp( gsm_buffer, command, len ) == 0 )
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
	gsm_capture_mask = 0;
	gsm_capture_state = 0;
	gsm_responseCREG = 'x';
}

/* Soft UART RX handler.
 * Each char is added to the gsm_buffer. If the char is \n we assume that a complete line has been received and it
 * will be parsed.
 * Parsing is done by looking for specific strings (only those specified in gsm_capture_mask are checked for) and setting the corresponding
 * flags or values.
 * The buffer is reset when the line is parsed.
 */
void swu_rx_callback(void)
{
	// Append to buffer
	unsigned char data = swu_rx_chr();
	gsm_buffer[ gsm_buffer_ptr++ ] = data;

	// When waiting for SMS prompt, we can't wait for \n since the prompt sequence is "\r\n> "
	if( gsm_capture_mask & GSMCAPTURE_SMSPROMPT )
		if( check_tail( "> " ))
		{
			gsm_capture_state |= GSMCAPTURE_SMSPROMPT;

			// Reset buffer pointer and we're done. (We don't even bother checking for overflow since the prompt follows a newline.)
			gsm_buffer_ptr = 0;
			return;
		}

	// Check for EOL
	if( data == '\n' )
	{
		// Yes, EOL. See what we're currently looking for and update the gsm_capture_state appropriately

		// OK\r\n
		if( gsm_capture_mask & GSMCAPTURE_OK )
			if( check_tail( "OK\r\n" ) )
				gsm_capture_state |= GSMCAPTURE_OK;

		// ERROR\r\n
		if( gsm_capture_mask & GSMCAPTURE_ERROR )
			if( check_tail( "ERROR\r\n" ) )
				gsm_capture_state |= GSMCAPTURE_ERROR;

		// CREG
		if( gsm_capture_mask & GSMCAPTURE_CREG )
			if( check_head( "+CREG: 0," ) )
				gsm_responseCREG = gsm_buffer[9];

		// The line has been handled: reset the counter
		gsm_buffer_ptr = 0;
	}


	// Wrap around if overflow
	gsm_buffer_ptr %= GSM_BUFFER_SIZE;
}

/* Sends a command and waits the specified time for either OK or ERROR response.
 * The command should *not* include the trailing \r
 * Return values: 0 = OK, 1 = ERROR received, 2 = timeout
 * Timeout value is specified in current SysTick intervals.
 */
uint8_t send_simplecommand( char *command, uint32_t timeout )
{
	// Clear response flags
	gsm_capture_state = 0;

	// We're waiting for OK or ERROR
	gsm_capture_mask = GSMCAPTURE_OK | GSMCAPTURE_ERROR;

	// Send command and terminating \r
	swu_tx_str( (unsigned char*)command );
	swu_tx_chr( '\r' );

	// Wait (block) until we've received OK or ERROR or timeout occurs
	SysTick_Reset();
	while( (gsm_capture_state & GSMCAPTURE_OK) == 0 && (gsm_capture_state & GSMCAPTURE_ERROR) == 0 && SysTick_Value < timeout )
		;

	// Return 1 if we got OK and 0 for timeout or ERROR
	if( (gsm_capture_state & GSMCAPTURE_ERROR) )
		return 1;
	if( SysTick_Value >= timeout )
			return 2;

	return 0;
}

/* Sends a text SMS to the specified recipient.
 * NOTE: the module must already have been configured for text messages using AT+CMGF=1
 * Return values: 0 = OK,
 * 1 = got ERROR after AT+CMGS=
 * 2 = timeout waiting for SMS prompt
 * 3 = got ERROR after sending message
 * 4 = timeout waiting for second SMS prompt
 * 5 = got ERROR after sending ctrl-Z
 * 6 = timeout after sending ctrl-Z
 */
uint8_t send_SMS( const char *recipient, const char *message )
{
	// Send AT+CMGS=+4526205528 and wait for SMS prompt
	gsm_capture_state = 0;
	gsm_capture_mask = GSMCAPTURE_SMSPROMPT | GSMCAPTURE_ERROR;		// Capture SMS prompt or ERROR
	swu_tx_str( (unsigned char*)"AT+CMGS=" );
	swu_tx_str( (unsigned char*)recipient );
	swu_tx_chr( '\r' );
	SysTick_Reset();

	// Wait (block) until we've received SMS prompt or ERROR or timeout occurs
	while( (gsm_capture_state & GSMCAPTURE_SMSPROMPT) == 0 && (gsm_capture_state & GSMCAPTURE_ERROR) == 0 && SysTick_Value < 40 )
		;

	// Return errors if applicable
	if( (gsm_capture_state & GSMCAPTURE_ERROR) )
		return 1;
	if( SysTick_Value >= 40 )
		return 2;

	// We got the prompt: send the message and terminate with \r\n
	gsm_capture_state = 0;
	swu_tx_str( (unsigned char*)message );
	swu_tx_chr( '\r' );
	SysTick_Reset();

	// Again, wait for prompt or ERROR
	while( (gsm_capture_state & GSMCAPTURE_SMSPROMPT) == 0 && (gsm_capture_state & GSMCAPTURE_ERROR) == 0 && SysTick_Value < 40 )
		;

	// Return errors if applicable
	if( (gsm_capture_state & GSMCAPTURE_ERROR) )
		return 3;
	if( SysTick_Value >= 40 )
		return 4;

	// Message sent: send ctrl-Z to terminate
	gsm_capture_state = 0;
	gsm_capture_mask = GSMCAPTURE_OK | GSMCAPTURE_ERROR;
	swu_tx_chr( 0x1A );
	SysTick_Reset();

	// Wait for OK/ERROR (60 secs timeout)
	while( (gsm_capture_state & GSMCAPTURE_OK) == 0 && (gsm_capture_state & GSMCAPTURE_ERROR) == 0 && SysTick_Value < 600 )
		;

	// Return errors if applicable
	if( (gsm_capture_state & GSMCAPTURE_ERROR) )
		return 5;
	if( SysTick_Value >= 600 )
		return 6;

	return 0;
}

void gsm_do_AT( char *args, char *output )
{
	uint8_t result = send_simplecommand( "AT", STANDARD_TIMEOUT );

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

/* Perform full init sequence.
 * The sequence is as follows:
 * 1) AT - to verify that we have a connection and that the GSM module is powered up. Timeout 2 secs.
 * 2) AT+CPIN=<SIM PIN code stored in EEPROM at offset 0> - timeout 5 secs.
 * 3) AT+CREG? - timeout 2 secs. Keep polling +CREG for 120 secs until we get either 1 (good, continue), 3 (bad, wrong network) or 0 (bad, possibly module reset due to power consumption)
 * 4) AT+CMGF=1 - configure SMS as text. Timeout 2 secs.
 * 5) AT+CNMI=3,2 - configure unsolicited SMS delivery. Timeout 2 secs.
 * ... (GPRS init sequence not yet implemented)
 */
void gsm_do_initsequence( char *args, char *output )
{
	// 1) AT
	uint8_t result = send_simplecommand( "AT", 20 );
	if( result != 0 )
	{
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #1\r\n" );
		return;
	}

	// 2) AT+CPIN=<pin>
	result = send_simplecommand( "AT+CPIN=8409", 50 );	// TODO: Use PIN from EEPROM
	if( result != 0 )
	{
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #2\r\n" );
		return;
	}

	// 3) Check AT+CREG until we get 0, 1 or 3
	SysTick_Reset();
	gsm_capture_mask = GSMCAPTURE_CREG | GSMCAPTURE_OK | GSMCAPTURE_ERROR;
	while( SysTick_Value < 1200 )	// Wait max 120 secs.
	{
		// Send AT+CREG? to check registration state
		gsm_capture_state = 0;
		gsm_responseCREG = 'x';
		swu_tx_str( (unsigned char*)"AT+CREG?\r" );
		uint32_t lastcregtimestamp = SysTick_Value;

		// Wait for reply or 2 seconds timeout (if we receive OK we have already captures CREG value).
		while( (gsm_capture_state & GSMCAPTURE_OK) == 0 && (gsm_capture_state & GSMCAPTURE_ERROR) == 0 && SysTick_Value < lastcregtimestamp+20 )
			;

		// Return 1 if we got OK and 0 for timeout or ERROR
		if( (gsm_capture_state & GSMCAPTURE_ERROR) )
		{
			sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #3\r\n" );
			return;
		}
		if( SysTick_Value >= lastcregtimestamp+20 )
		{
			sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #4\r\n" );
			return;
		}

		// See if we have a valid CREG value
		/*
		if( gsm_responseCREG == '0' )
		{
			sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #5\r\n" );
			return;
		}
		*/
		else if( gsm_responseCREG == '3' )
		{
			sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #6\r\n" );
			return;
		}
		else if( gsm_responseCREG == '1' )
			// We are registered: break out of while-loop and continue
			break;

		// Wait 10 secs and try again
		lastcregtimestamp = SysTick_Value;
		while( SysTick_Value < lastcregtimestamp+100 )
			;
	}

	// If we're not registered now, exit
	if( gsm_responseCREG == '0' )
	{
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #5\r\n" );
		return;
	}

	// Check for 120 secs timeout
	if( SysTick_Value > 1200 )
	{
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #7\r\n" );
		return;
	}
	// We're good: continue

	// 4) Configure SMS for text messages
	result = send_simplecommand( "AT+CMGF=1", 20 );
	if( result != 0 )
	{
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - init error #8\r\n" );
		return;
	}

	// 5) Configure for unsolicited SMS text message delivery
	// TODO: AT+CNMI=3,2

	// TODO: Configure GPRS

	// We're done
	sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - init complete\r\n" );
}

/* Sends an SMS.
 * GSM module must be configured and registered first.
 */
void gsm_do_SMS( char *args, char *output )
{
	// Find comma in args
	char *recipient = args;
	char *message = strchr( args, ',' );

	// If we didn't find the comma, return error message
	if( message == 0 )
	{
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR - format must be 'GSM SMS=<recipient>,<message>'" );
		return;
	}

	// Replace comma with \0 to terminate recipient string and advance message to next character
	*message = 0;
	message++;

	// Send it
	uint8_t result = send_SMS( recipient, message );
	if( result == 0 )
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - SMS sent\r\n" );
	else
		sniprintf( output, OUTPUT_BUFFER_SIZE, "ERROR sending SMS: %d\r\n", result );
}
