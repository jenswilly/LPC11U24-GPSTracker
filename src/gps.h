/*
 * gps.h
 *
 *  Created on: Aug 24, 2012
 *      Author: jenswilly
 */

#ifndef GPS_H_
#define GPS_H_

extern char *gps_buffer;
extern uint8_t gps_rawecho;

void gps_init( void );
void gps_do_init( char *args, char *output );
void gps_do_rawecho( char *args, char *output );

#endif /* GPS_H_ */
