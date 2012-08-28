/*
 * gps.h
 *
 *  Created on: Aug 24, 2012
 *      Author: jenswilly
 */

#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>

typedef struct
{
	char lat[11];			// Latitude as string in DDMM.MMMMX format (X=N/S)
	char lng[12];			// Longitude as string in DDDMM.MMMMX format (X=E/W)
	char timestamp[11];		// Timestamp string in HHMMSS.SSS format
	uint8_t status;			// 0=no fix; 1=position fix
	uint8_t sats_in_view;	// Number of satellites currently reported in view
	uint8_t sats_tracking;	// Number of satellites with fix
} GPSStatus;

// Global vars
extern GPSStatus gps_status;
extern uint8_t gps_rawecho;

// Function prototypes
void gps_init( void );
void gps_do_status( char *args, char *output );
void gps_do_init( char *args, char *output );
void gps_do_rawecho( char *args, char *output );
void parse_RMC( void );
void parse_GSV( void );

#endif /* GPS_H_ */
