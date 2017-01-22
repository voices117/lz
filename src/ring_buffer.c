/* include area */
#include "ring_buffer.h"
#include "math2.h"


/* implementations */

/**
 * Initializes a ring buffer with the given size.
 * @param  rb   Ring buffer to initialize.
 * @param  size Size of the ring buffer.
 * @return      \c true on success, \c false otherwise.
 */
bool ring_buffer_init( ring_buffer_t *rb, size_t size )
{
  if( size == 0 )
    return false;

  rb->buffer = malloc( size );
  if( rb->buffer == NULL )
    return false;

  rb->size = size;
  ring_buffer_reset( rb );

  /* success */
  return true;
}


/**
 * Releases resources taken by the ring buffer initialization.
 * @param rb Ring buffer to release.
 */
void ring_buffer_release( ring_buffer_t *rb )
{
  free( rb->buffer );
  rb->buffer = NULL;
}


/**
 * Appends a new byte to the ring buffer.
 * If the ring buffer is already full, the oldest byte is discarded.
 * @param rb Ring buffer to append the byte to.
 * @param b  Byte to append.
 */
void ring_buffer_append( ring_buffer_t *rb, byte b )
{
  rb->buffer[rb->bytes_count % rb->size] = b;
  rb->bytes_count += 1;
}


/**
 * Gets a byte at a determined position from the ring buffer.
 * @param  rb  Ring buffer to get the byte from.
 * @param  b   Byte read (output).
 * @param  pos Position to read from.
 * @return     \c true is the byte was correctly read, \c false otherwise.
 *
 * \note if the position trying to be read was discarded (or not added yet), the return value will
 *       be \c false.
 */
bool ring_buffer_get( const ring_buffer_t *rb, byte *b, uint64_t pos )
{
  if( pos >= rb->bytes_count )
    return false;
  else if ( rb->bytes_count > rb->size && pos < ( rb->bytes_count - rb->size ) )
    return false;

  *b = rb->buffer[pos % rb->size];
  return true;
}


/**
 * Returns the first position with available bytes to be read from the ring buffer.
 * @param  rb Ring buffer.
 * @return    Position with the first available byte.
 */
uint64_t ring_buffer_first_pos( const ring_buffer_t *rb )
{
  return MAX( ( int64_t )( rb->bytes_count - rb->size ), 0 );
}


/**
 * Resets the ring buffer.
 * @param rb ring buffer.
 */
void ring_buffer_reset( ring_buffer_t *rb )
{
  rb->start = rb->buffer;
  rb->bytes_count = 0;
}
