#include <string.h>
#include "math2.h"
#include "scunit.h"
#include "window.h"


/*
   4 3 2 1 0   (position)
  | | | | |a|  <-- append an 'a'
  -----------
  | | | |a|b|  <-- append a 'b'
  -----------
  | | |a|b|c|  <-- append a 'c'
  -----------
  | |a|b|c|d|  <-- append a 'd'
  -----------
  |a|b|c|d|e|  <-- append a 'e'
  -----------
  |b|c|d|e|f|  <-- append a 'f'

  ... etc

 */


TEST( BasicTest )
{
  #define WINDOW_SIZE 50

  char c;
  window_t w;

  ASSERT_TRUE( window_init( &w, WINDOW_SIZE ) );
  ASSERT_EQ( 0, window_get_size( &w ) );
  ASSERT_FALSE( window_read( &w, &c, 0 ) );

  /* fills the window */
  for( size_t i = 0; i < WINDOW_SIZE; i++ )
  {
    window_append( &w, i );
    ASSERT_EQ( i + 1, window_get_size( &w ) );

    for( size_t j = 0; j <= i; j++ )
    {
      ASSERT_TRUE( window_read( &w, &c, j ) );
      ASSERT_EQ( i - j, c );
    }
  }

  /* now checks the overflow case */
  window_append( &w, 'z' );
  ASSERT_EQ( WINDOW_SIZE, window_get_size( &w ) );

  ASSERT_FALSE( window_read( &w, &c, WINDOW_SIZE ) );

  /* the original first element has now fallen out of the window */
  ASSERT_TRUE( window_read( &w, &c, WINDOW_SIZE - 1 ) );
  ASSERT_EQ( 1, c );

  /* checks the last added element */
  ASSERT_TRUE( window_read( &w, &c, 0 ) );
  ASSERT_EQ( 'z', c );

  window_release( &w );

  #undef WINDOW_SIZE
}


TEST( Overflow )
{
  #define WINDOW_SIZE 15000
  #define BYTES_TO_WRITE ( WINDOW_SIZE * 3 )

  const char input[] = "Constipated people don't give a crap.";

  char c;
  window_t w;

  window_init( &w, WINDOW_SIZE );

  /* fills the window and checks the last character */
  for( size_t i = 0; i < BYTES_TO_WRITE; i++ )
  {
    char new = input[i % strlen( input )];

    window_append( &w, new );
    ASSERT_EQ( MIN( i + 1, WINDOW_SIZE ), window_get_size( &w ) );

    ASSERT_TRUE( window_read( &w, &c, 0 ) );

    char expected = new;
    ASSERT_EQ( expected, c );
  }

  /* calculates the number of truncated characters from the last sentence */
  size_t unwritten = BYTES_TO_WRITE % ( ( BYTES_TO_WRITE / strlen( input ) ) * strlen( input ) );

  /* finds the beginning of the last sentence in the window that fitted entirely */
  size_t idx = strlen( input ) + unwritten - 1;

  /* checks the whole string is retrieved */
  for( size_t i = 0; i < strlen( input ); i++ )
  {
    ASSERT_TRUE( window_read( &w, &c, idx - i ) );
    ASSERT_EQ( input[i], c );
  }

  window_release( &w );

  #undef WINDOW_SIZE
}
