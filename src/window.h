#ifndef WINDOW_H
#define WINDOW_H


/* include area */
#include <stdlib.h>
#include <stdbool.h>
#include "ring_buffer.h"


/** Library data types. */

/** Window type.  */
typedef struct
{
  /** Internal buffer. */
  ring_buffer_t rb;

  /** Internal buffer's size. */
  size_t buffer_size;

  /** Number of bytes already written. */
  uint64_t data_size;

} window_t;


/** Prototypes */
bool window_init( window_t *w, size_t size );
void window_release( window_t *w );

/* IO */
void window_append( window_t *w, char c );
bool window_read( const window_t *w, char *c, size_t pos );

/* misc */
size_t window_get_size( const window_t *w );
void window_clear( window_t *w );


#endif
