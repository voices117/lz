#ifndef WINDOW_H
#define WINDOW_H


/* include area */
#include <stdlib.h>
#include <stdbool.h>


/** Library data types. */

/** Window type.  */
typedef struct
{
  /** Internal buffer. */
  char *buffer;

  /** Internal buffer's size. */
  size_t buffer_size;

  /** Offset to the logical beginning of the buffer (from the physical address). */
  size_t start_offset;

  /** Number of bytes already occupied. */
  size_t data_size;

} window_t;


/** Prototypes */
bool window_create( window_t *w, size_t size );
void window_destroy( window_t *w );

/* IO */
void window_append( window_t *w, char c );
bool window_read( const window_t *w, char *c, size_t pos );

/* misc */
size_t window_get_size( const window_t *w );
void window_clear( window_t *w );


#endif
