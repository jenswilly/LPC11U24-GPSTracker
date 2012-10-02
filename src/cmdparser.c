/*
 * cmdparser.c
 *
 *  Created on: Aug 16, 2012
 *      Author: jenswilly
 */

#include "cmdparser.h"
#include <strings.h>
#include <stdio.h>

extern void USB_CDC_print( char* string );	// From main.c

char response_buffer[ OUTPUT_BUFFER_SIZE ];

typedef struct
{
	const char *command_name;			// Name of command
	uint8_t permissions;				// Permissions flags
	void (*handler)(char*, char*);		// Handler function of type void handler( uint8_t* args, uint8_t* output )
} Command;

typedef struct
{
	const char *component_name;			// Name of component
	const Command *command_table;		// Pointer to the command table (array of Command structures)
} Component;

const Command sys_commands[] =
{
	{"test",	0,	sys_do_testcmd },
	{"version",	0,	sys_do_version },
	{"eeprom",	0,	sys_do_eeprom },
	{0,0,0}
};

const Command gsm_commands[] =
{
	{"at",		0, gsm_do_AT },
	{"pin",		0, gsm_do_pin },
	{"raw",		0, gsm_do_raw },
	{"sms",		0, gsm_do_SMS },
	{"init",	0, gsm_do_initsequence },
	{0,0,0}
};

const Command gps_commands[] =
{
	{"init", 	0, gps_do_init },		// GPS INIT
	{"echo", 	0, gps_do_rawecho },	// GPS ECHO[=1/0]
	{"status",	0, gps_do_status },		// GPS STATUS
	{0,0,0}
};

const Component components[] =
{
	{"sys", 	sys_commands },
	{"gsm", 	gsm_commands },
	{"gps", 	gps_commands },
	{0,0}
};

/* Returns a pointer to a command structure matching the specified name in the specified command table.
 */
const Command* findcommand( char *command_name, const Command *command_table )
{
	// Iterate all commands in the command table
	const Command *command = command_table;
	while( command->command_name )
	{
		// See if name matches
		if( strcasecmp( command_name, command->command_name ) == 0 )
			// Found it: return the matching command structure
			return command;

		command++;
	}

	// Didn't find any: return 0
	return 0;
}

/* Returns a pointer to a component structure based on the component name.
 * If not found, 0 is returned.
 */
const Component* findcomponent( char *component_name, const Component *component_table )
{
	// Iterate all components
	const Component *component = component_table;

	while( component->component_name )
	{
		// See if name matches
		if( strcasecmp( component_name, component->component_name ) == 0 )
			// Found it: return the matching component structure
			return component;

		component++;
	}

	// Didn't find any: return 0
	return 0;
}

/*
void testcommandparse()
{
	char *str_component = "SYS";
	char *str_command = "test";

	// See if we can match the component
	Component *component = findcomponent( str_component, components );

	// If we didn't find any, return
	if( !component )
		return;

	// We got the component; see if we can find the command
	Command *command = findcommand( str_command, component->command_table );

	// If we didn't find any, return
	if( !command )
		return;

	// Invoke function
	command->handler( 0, response_buffer );
}
*/

int parsecommandline( uint8_t* buffer )
{
	char *str_component = 0;
	char *str_command = 0;
	char *str_args = 0;

	// Component starts at offset 0
	str_component = (char*)buffer;

	// Skip ahead until we find a space
	while( *buffer != ' ' )
		buffer++;

	// Replace the space with \0 to terminate the component string and increase pointer
	*buffer++ = 0;

	// Pointer is now at beginning of command
	str_command = (char*)buffer;

	// Skip until we find =, CR, LF or EOS
	while( *buffer && *buffer != '=' && *buffer != '\r' && *buffer != '\n' )
		buffer++;

	// Did we find =?
	if( *buffer == '=' )
	{
		// Yes: terminate command string
		*buffer++ = 0;

		// Set args to next char
		str_args = (char*)buffer;

		// And skip ahead until we find \n, \r or EOS
		while( *buffer != '\r' && *buffer != '\n' && buffer )
			buffer++;

		// Terminate args string
		*buffer = 0;
	}
	else
	{
		// Didn't find = so just terminate string here. str_args will be 0.
		*buffer = 0;
	}

	// See if we can match the component
	const Component *component = findcomponent( str_component, components );
/*
	sniprintf( response_buffer, OUTPUT_BUFFER_SIZE, "Component: %s, ptr: %p, ptr->name: %s\r\n", str_component, component, component->component_name );
	USB_CDC_print( response_buffer );
	return 0;
*/
	// If we didn't find any, return
	if( !component )
		return 1;

	// We got the component; see if we can find the command
	const Command *command = findcommand( str_command, component->command_table );

	// If we didn't find any, return
	if( !command )
		return 2;

	// Check permissions
	// TODO: check permissions according to permissions flag and sender

	// Invoke function
	command->handler( str_args, response_buffer );

	// And output result
	USB_CDC_print( response_buffer );

	return 0;
}

