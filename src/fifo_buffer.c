/*
 * fifo_buffer.c
 *
 *  Created on: Sep 2, 2012
 *      Author: jenswilly
 *
 *  Circluar FIFO buffer.
 */

#include "fifo_buffer.h"

/* Resets the buffer counters.
 */
void fifobuffer_reset( FIFOBuffer *buffer )
{
	buffer->start = 0;
	buffer->count = 0;
}

/* Returns whether or not the specified buffer is full.
 */
uint8_t fifobuffer_isfull( FIFOBuffer *buffer )
{
	return (buffer->count == FIFO_BUFFER_SIZE );
}

/* Returns whether or not the specified buffer is empty.
 */
uint8_t fifobuffer_ifempty( FIFOBuffer *buffer )
{
	return (buffer->count == 0);
}

/* Reads up to length bytes from the specified buffer into the destination buffer.
 * The actual number of read bytes is returned.
 */
uint32_t fifobuffer_read( FIFOBuffer *buffer, uint8_t *destination, uint32_t length )
{
	uint32_t read = 0;

	while( buffer-> count && length )
	{
		destination[ read++ ] = buffer->data[ buffer->start++ ];

		// Wrap around if necessary
		buffer->start &= FIFO_BUFFER_SIZE-1;

		buffer->count--;
		length--;
	}
	// Return the actual number of bytes read.
	return read;
}

/* Writes up to length bytes from source to the buffer data.
 * The actual number of written bytes is returned.
 */
uint32_t fifobuffer_write( FIFOBuffer *buffer, uint8_t *source, uint32_t length )
{
	uint32_t written = 0;

	while( buffer->count < FIFO_BUFFER_SIZE && length )
	{
		buffer->data[ (buffer->start + buffer->count) & (FIFO_BUFFER_SIZE-1) ] = source[ written++ ];

		buffer->count++;
		length--;
	}

	// Return the number of bytes written
	return written;
}
