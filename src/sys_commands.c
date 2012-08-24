/*
 * sys_commands.c
 *
 *  Created on: Aug 17, 2012
 *      Author: jenswilly
 */

#include "sys_commands.h"
#include "cmdparser.h"		// For OUTPUT_BUFFER_SIZE
#include <stdio.h>
#include "version.h"
#include "eeprom.h"

/* Writes back a simple test string.
 */
void sys_do_testcmd( char *args, char *output )
{
	sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - Response from SYS TEST\r\n" );
}

/* Returns (i.e. filles the provided output buffer with) firmware name and current info.
 */
void sys_do_version( char *args, char *output )
{
	sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - b%s on %s, git %s\r\n", build_number, build_date, build_git_sha );
}

/* Reads or sets EEPROM values. No args=show contents; args=store contents.
 * Fours bytes are stored at EEPROM offset 0.
 */
void sys_do_eeprom( char *args, char *output )
{
	// Arguments?
	if( args == 0 )
	{
		// No arguments: display contents
		char eeprom[4] = "----";
		readEEPROM( (uint8_t*)0, (uint8_t*)eeprom, 4 );		// Read 4 bytes from EEPROM offset 0
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - EEPROM contents: %s\r\n", eeprom );
	}
	else
	{
		// Arguments: write first four bytes
		writeEEPROM( (uint8_t*)0, (uint8_t*)args, 4 );	// Write 4 bytes to EEPROM offset 0
		sniprintf( output, OUTPUT_BUFFER_SIZE, "OK - EEPROM set: %s\r\n", args );
	}
}
