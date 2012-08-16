/*
 * sys_commands.c
 *
 *  Created on: Aug 17, 2012
 *      Author: jenswilly
 */

#include "sys_commands.h"
extern void USB_CDC_print( char* string );	// From main.c

void do_testcmd( uint8_t *args )
{
	// Print msg
	USB_CDC_print( "Response from SYS TEST\r\n" );
}
