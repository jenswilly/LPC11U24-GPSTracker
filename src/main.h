/*
 * main.h
 *
 *  Created on: Aug 24, 2012
 *      Author: jenswilly
 */

#ifndef MAIN_H_
#define MAIN_H_

enum
{
	StateIdle,
	StateCmdReceived,		// Entire command received: go parse it
	StateGPSLineReceived,	// Line received from GPS: parse it
	StateGPSParseGSV,
	StateGPSParseRMC
};

// Global vars
extern volatile uint16_t state;

// Function prototypes
void USB_CDC_print( char* string );

#endif /* MAIN_H_ */
