/*
 * cmdparser.c
 *
 *  Created on: Aug 16, 2012
 *      Author: jenswilly
 */

#include "cmdparser.h"
#include <strings.h>

typedef struct
{
	char *command_name;			// Name of command
	uint8_t permissions;		// Permissions flags
	void (*handler)(uint8_t*);	// Handler function of type void handler( uint8_t* args )
} Command;

typedef struct
{
	char *component_name;		// Name of component
	Command *command_table;		// Pointer to the command table (array of Command structures)
} Component;

static Command sys_commands[] =
{
	{"restart",	0,	do_testcmd },
	{"pwr",		0,	do_testcmd },
	{0,0,0}
};

static Command gsm_commands[] =
{
	{"stat",	0,	do_testcmd },
	{0,0,0}
};

static Command gps_commands[] =
{
	{"stat",	0,	do_testcmd },
	{0,0,0}
};

static Component components[] =
{
	{"sys", 	sys_commands },
	{"gsm", 	gsm_commands },
	{"gps", 	gps_commands },
	{0,0}
};

/* Returns a pointer to a command structure matching the specified name in the specified command table.
 */
Command* findcommand( char *command_name, Command *command_table )
{
	// Iterate all commands in the command table
	Command *command = command_table;
	while( command->command_name )
	{
		// See if name matches
		if( strcasecmp( command_name, command->command_name ) == 0 )
			// Found it: return the matching command structure
			return command;
	}

	// Didn't find any: return 0
	return 0;
}

/* Returns a pointer to a component structure based on the component name.
 * If not found, 0 is returned.
 */
Component* findcomponent( char *component_name, Component *component_table )
{
	// Iterate all components
	Component *component = component_table;
	while( component->component_name )
	{
		// See if name matches
		if( strcasecmp( component_name, component->component_name ) == 0 )
			// Found it: return the matching component structure
			return component;
	}

	// Didn't find any: return 0
	return 0;
}

void testcommandparse()
{
	char *str_component = "SYS";
	char *str_command = "reset";

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
	command->handler( 0 );
}
