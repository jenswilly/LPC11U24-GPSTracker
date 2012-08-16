/*
 * cmdparser.h
 *
 *  Created on: Aug 16, 2012
 *      Author: jenswilly
 */

#ifndef __CMDPARSER_H
#define __CMDPARSER_H

#include <stdint.h>

void testcommandparse(void);


#define CMD(name)	extern void name( uint8_t* parameter );

// SYS commands
CMD( do_testcmd );
CMD( do_restart_all );
CMD( do_pwr_report );

// GSM commands
CMD( do_gsm_stat );

// GPS commands
CMD( do_gps_stat );

#endif
