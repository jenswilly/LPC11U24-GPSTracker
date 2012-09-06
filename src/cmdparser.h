/*
 * cmdparser.h
 *
 *  Created on: Aug 16, 2012
 *      Author: jenswilly
 */

#ifndef __CMDPARSER_H
#define __CMDPARSER_H

#include <stdint.h>

// Size of the output buffer. Use the sniprintf() or snprintf() function to be sure not to exceed the buffer size
#define OUTPUT_BUFFER_SIZE 100
extern char response_buffer[];


void testcommandparse( void );
int parsecommandline( uint8_t* buffer );

#define CMD(name)	extern void name( char* parameter, char* output_buffer );

// SYS commands
CMD( sys_do_testcmd );
CMD( sys_do_version );
CMD( sys_do_eeprom );

// GSM commands
CMD( gsm_do_AT );
CMD( gsm_do_pin );
CMD( gsm_do_raw );

// GPS commands
CMD( gps_do_init );
CMD( gps_do_rawecho );
CMD( gps_do_status );

#endif
