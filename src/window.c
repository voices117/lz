#include <string.h>
#include "math2.h"
#include "window.h"


/**
 * Initializes a window.
 *
 * @param  w           Window to initialize.
 * @param  size        Size of the window.
 * @return             On success \c true, \c false otherwise.
 */
bool window_init( window_t *w, size_t size )
{
  if( !ring_buffer_init( &w->rb, size ) )
    return false;

  w->buffer_size = size;
  w->data_size = 0;

  return true;
}


/**
 * Destroys a window, releasing all resources.
 *
 * @param w Window.
 */
void window_release( window_t *w )
{
  ring_buffer_release( &w->rb );
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
  w->data_size += 1;
  ring_buffer_append( &w->rb, c );
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
  uint64_t rb_pos = ( w->data_size - pos ) - 1;
  return ring_buffer_get( &w->rb, ( byte * )c, rb_pos );
}


/**
 * Returns the number of bytes contained in the window.
 * @param  w Window.
 * @return   Number of characters contained.
 */
size_t window_get_size( const window_t *w )
{
  return MIN( w->data_size, w->buffer_size );
}


/**
 * Removes all characters stored in the window.
 * @param w Window.
 */
void window_clear( window_t *w )
{
  w->data_size = 0;
  ring_buffer_reset( &w->rb );
}
