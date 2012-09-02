/*
 * fif0_buffer.h
 *
 *  Created on: Sep 2, 2012
 *      Author: jenswilly
 */

#ifndef FIFO_BUFFER_H_
#define FIFO_BUFFER_H_

#include <stdint.h>

#define FIFO_BUFFER_SIZE 256	// Must be power of 2 (since we're using & SIZE-1 to wrap around Ð otherwise, change to %)

// Circular buffer struct
typedef struct
{
	uint8_t data[ FIFO_BUFFER_SIZE ];
	volatile uint32_t start;
	volatile uint32_t count;
} FIFOBuffer;

void fifobuffer_reset( FIFOBuffer *buffer );
uint8_t fifobuffer_isfull( FIFOBuffer *buffer );
uint8_t fifobuffer_ifempty( FIFOBuffer *buffer );
uint32_t fifobuffer_read( FIFOBuffer *buffer, uint8_t *destination, uint32_t length );
uint32_t fifobuffer_write( FIFOBuffer *buffer, uint8_t *source, uint32_t length );

#endif /* FIFO_BUFFER_H_ */
