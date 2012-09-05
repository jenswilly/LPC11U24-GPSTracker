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
void gms_init( void );

// GSM commands
void gsm_do_AT( char *args, char *output );

#endif /* GSM_H_ */
