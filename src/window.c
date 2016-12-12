#include <string.h>
#include "window.h"


/**
 * Initializes a window.
 *
 * @param  w           Window to initialize.
 * @param  size        Size of the window.
 * @return             On success \c true, \c false otherwise.
 */
bool window_create( window_t *w, size_t size )
{
  w->buffer = malloc( size );
  if( w->buffer == NULL )
    return false;

  w->buffer_size = size;
  w->start_offset = 0;
  w->data_size = 0;

  return true;
}


/**
 * Destroys a window, releasing all resources.
 *
 * @param w Window.
 */
void window_destroy( window_t *w )
{
  free( w->buffer );
  memset( w, 0, sizeof( *w ) );
}


/**
 * Appends a new character into the window.
 *
 * @param  w Window.
 * @param  c Character to append.
 * @return   Number of matches.
 */
void window_append( window_t *w, char c )
{
  /* if the buffer is not complete yet, just appends the character */
  if( w->data_size < w->buffer_size )
  {
    w->buffer[w->data_size] = c;
    w->data_size++;
    return;
  }

  /* if it's already full, overwrites the oldest element and updates the start offset */
  w->buffer[w->start_offset] = c;
  if( ++w->start_offset == w->buffer_size )
    w->start_offset = 0;
}


/**
 * Reads a character from the window at a given position.
 *
 * @param  w   Window.
 * @param  c   Output character.
 * @param  pos Potion in the window to read from.
 * @return     On success \c true, or \c false if the position is invalid.
 */
bool window_read( const window_t *w, char *c, size_t pos )
{
  if( pos >= w->data_size )
    return false;

  /* adjusts the position to the logical address */
  pos = ( w->start_offset + pos ) % w->buffer_size;
  *c = w->buffer[pos];

  return true;
}


/**
 * Returns the number of bytes contained in the window.
 * @param  w Window.
 * @return   Number of characters contained.
 */
size_t window_get_size( const window_t *w )
{
  return w->data_size;
}


/**
 * Removes all characters stored in the window.
 * @param w Window.
 */
void window_clear( window_t *w )
{
  w->data_size = 0;
  w->start_offset = 0;
}
