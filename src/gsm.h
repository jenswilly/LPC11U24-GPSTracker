/*
 * gsm.h
 *
 *  Created on: Sep 5, 2012
 *      Author: jenswilly
 */

#ifndef GSM_H_
#define GSM_H_

#include <stdint.h>

// Function prototypes
void gsm_init( void );

// GSM commands
void gsm_do_AT( char *args, char *output );
void gsm_do_pin( char *args, char *output );
void gsm_do_raw( char *args, char *output );

#endif /* GSM_H_ */
