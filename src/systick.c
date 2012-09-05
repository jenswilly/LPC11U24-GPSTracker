/*
 * systick.c
 *
 *  Created on: Sep 4, 2012
 *      Author: jenswilly
 */
#include "systick.h"

volatile uint32_t SysTick_Value;

void SysTick_Handler( void )
{
	// Increase counter. Roll-over might occur
	SysTick_Value++;
}

void SysTick_Delay( uint32_t ticks )
{
	// Reset the counter
	SysTick_Reset();

	while( SysTick_Value < ticks )
		;
}
