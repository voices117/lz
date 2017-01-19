#include <string.h>
#include "math2.h"
#include "scunit.h"
#include "window.h"


TEST( BasicTest )
{
  #define WINDOW_SIZE 50

  char c;
  window_t w;

  ASSERT_TRUE( window_create( &w, WINDOW_SIZE ) );
  ASSERT_EQ( 0, window_get_size( &w ) );
  ASSERT_FALSE( window_read( &w, &c, 0 ) );

  /* fills the window */
  for( size_t i = 0; i < WINDOW_SIZE; i++ )
  {
    window_append( &w, i );
    ASSERT_EQ( i + 1, window_get_size( &w ) );

    ASSERT_TRUE( window_read( &w, &c, i ) );
    ASSERT_EQ( i, c );
  }

  /* now checks the overflow case */
  window_append( &w, 'z' );
  ASSERT_EQ( WINDOW_SIZE, window_get_size( &w ) );

  ASSERT_FALSE( window_read( &w, &c, WINDOW_SIZE ) );

  /* the original first element has now fallen out of the window */
  ASSERT_TRUE( window_read( &w, &c, 0 ) );
  ASSERT_EQ( 1, c );

  /* checks the last element */
  ASSERT_TRUE( window_read( &w, &c, WINDOW_SIZE - 1 ) );
  ASSERT_EQ( 'z', c );

  window_destroy( &w );

  #undef WINDOW_SIZE
}


TEST( Overflow )
{
  #define WINDOW_SIZE 15000

  const char input[] = "Constipated people don't give a crap.";

  char c;
  window_t w;

  window_create( &w, WINDOW_SIZE );

  /* fills the window and checks the last character */
  for( size_t i = 0; i < WINDOW_SIZE * 3; i++ )
  {
    window_append( &w, input[i % strlen( input )] );
    ASSERT_EQ( MIN( i + 1, WINDOW_SIZE ), window_get_size( &w ) );

    size_t index = MIN( i, WINDOW_SIZE - 1 );
    ASSERT_TRUE( window_read( &w, &c, index ) );

    char expected = input[i % strlen( input )];
    ASSERT_EQ( expected, c );
  }

  /* finds the beginning of the last sentence in the window that fitted entirely */
  size_t idx = ( ( ( ( WINDOW_SIZE * 3 ) / strlen( input ) ) - 1 ) * strlen( input ) ) % WINDOW_SIZE;

  /* checks the whole string is retrieved */
  for( size_t i = 0; i < strlen( input ); i++ )
  {
    ASSERT_TRUE( window_read( &w, &c, i + idx ) );
    ASSERT_EQ( input[i], c );
  }

  window_destroy( &w );

  #undef WINDOW_SIZE
}
