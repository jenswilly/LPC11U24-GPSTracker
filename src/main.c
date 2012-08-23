/* 
 *  main.c
 *  
 *  Created by Jens Willy Johannsen on 2012-08-14.
 *
 *  This file is released under Creative Commons - Attribution 3.0 Unported (CC BY 3.0)
 *  http://creativecommons.org/licenses/by/3.0/
 *
 */

#include <string.h>
#include <stdint.h>
#include "LPC11Uxx.h"            
#include "usb_cdc.h"
#include "eeprom.h"
#include "cmdparser.h"

enum
{
	StateIdle,
	StateCmdReceived,	// Entire command received: go parse it
};

#define CMD_BUF_SIZE 200
uint8_t commandBuffer[CMD_BUF_SIZE];
volatile uint16_t commandBufferPtr;
volatile uint16_t state;

/* Utility function for printing USB messages.
 */
void USB_CDC_print( char* string )
{
	USB_CDC_send( (uint8_t*)string, strlen( string ));
//	USB_CDC_send( (uint8_t*)"\r\n", 2 );
}

/* Parse the command in the command buffer and perform required operations.
 */
void parseCommand(void)
{
	if( strncasecmp( "show eeprom", (char*)commandBuffer, 11 ) == 0 )
	{
		// TEMP: show EEPROM contents
		char output[] = "EEPROM contents:----\r\n";
		readEEPROM( (uint8_t*)0, (uint8_t*)output+16, 4 );		// Read 4 bytes from EEPROM offset 0 into output[16] to [19]

		USB_CDC_send( (uint8_t*)output, 22 );
	}
	else if( strncasecmp( "set eeprom=", (char*)commandBuffer, 11 ) == 0 )
	{
		// TEMP: write EEPROM
		writeEEPROM( (uint8_t*)0, (uint8_t*)commandBuffer+11, 4 );	// Write 4 bytes to EEPROM offset 0 from commandBuffer+11
		USB_CDC_print( "EEPROM set.\r\n" );
	}
	else
	{
		if( parsecommandline( commandBuffer ) != 0 )
			USB_CDC_print( "ERROR in command parser.\r\n" );
	}
	/*
	else
	{
		// Unknown command
		USB_CDC_print( "ERROR unknown command\r\n" );
	}
	*/

	// Reset command buffer and state
	commandBufferPtr = 0;
	state = StateIdle;
}

int main (void)
{
    SystemCoreClockUpdate ();
    USB_CDC_init();

    // Init state machine
    state = StateIdle;

    // Init command buffer
    memset( commandBuffer, CMD_BUF_SIZE, 0 );
    commandBufferPtr = 0;

    // Send "ready"
 //   USB_CDC_send( (uint8_t*)"Ready...\r\n", 10 );

	// Empty main loop
	while( 1 )
	{
		switch( state )
		{
			case StateCmdReceived:
				parseCommand();
				break;

			default:
				break;
		}
	}
}

void USB_CDC_receive( uint8_t *bufferPtr, uint32_t length )
{
	// Copy into command buffer
	// Prevent buffer overflow
	if( commandBufferPtr + length > CMD_BUF_SIZE )
	{
		// Overflow: ignore this entire chunk (since it will be invalid anyway) and send error
		USB_CDC_print( "ERROR command buffer overflow!\r\n" );

		// Reset command buffer
		commandBufferPtr = 0;
		return;
	}

	// No overflow: copy data into command buffer
	memcpy( commandBuffer+commandBufferPtr, bufferPtr, length );
	commandBufferPtr += length;

	// Check if we have received \n to terminate command
	if( commandBuffer[ commandBufferPtr-1 ] == '\n' )
		// Yes, we have: parse the command
		state = StateCmdReceived;

	// Data received: echo
//	USB_CDC_send( bufferPtr, length );
}
