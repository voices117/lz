#ifndef RING_BUFFER_H
#define RING_BUFFER_H


/* include area */
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "datatype.h"


/** Ring buffer type */
typedef struct
{
  /** Pointer to the internal buffer. */
  byte *buffer;

  /** Ring buffer size. */
  size_t size;

  /** Pointer to the oldest byte. */
  byte *start;

  /** Bytes written into the ring buffer. */
  uint64_t bytes_count;

} ring_buffer_t;


/* prototypes */
bool ring_buffer_init( ring_buffer_t *rb, size_t size );
void ring_buffer_release( ring_buffer_t *rb );

/* IO */
void ring_buffer_append( ring_buffer_t *rb, byte b );
bool ring_buffer_get( const ring_buffer_t *rb, byte *b, uint64_t pos );

/* misc */
uint64_t ring_buffer_first_pos( const ring_buffer_t *rb );
void ring_buffer_reset( ring_buffer_t *rb );


#endif
