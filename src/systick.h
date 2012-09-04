/*
 * systick.h
 *
 *  Created on: Sep 4, 2012
 *      Author: jenswilly
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include <stdint.h>                      /*!< standard types definitions                      */

extern volatile uint32_t SysTick_Value;	// SysTick counter value

void SysTick_Delay( uint32_t ticks );	// Blocking delay functions. Returns after 'ticks' systicks have occurred. The value is reset first to prevent roll-over problems.

// Resets the counter
static inline void SysTick_Reset( void )
{
	SysTick_Value = 0;
}

#endif /* SYSTICK_H_ */
