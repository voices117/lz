#include <string.h>
#include "scunit.h"
#include "ring_buffer.h"


TEST( Empty )
{
  ring_buffer_t rb;
  size_t rb_size = 10;

  ASSERT_TRUE( ring_buffer_init( &rb, rb_size ) );

  /* first available position is zero */
  ASSERT_EQ( 0, ring_buffer_first_pos( &rb ) );

  byte obtained;
  for( size_t i = 0; i < rb_size * 2; i++ )
    ASSERT_FALSE( ring_buffer_get( &rb, &obtained, i ) );

  ring_buffer_release( &rb );
}


TEST( AppendBytes )
{
  ring_buffer_t rb;
  size_t rb_size = 10;

  ASSERT_TRUE( ring_buffer_init( &rb, rb_size ) );

  /* sets the ring buffer full */
  for( size_t i = 0; i < rb_size; i++ )
  {
    ring_buffer_append( &rb, i );

    byte obtained;
    ASSERT_TRUE( ring_buffer_get( &rb, &obtained, i ) );

    ASSERT_EQ( i, obtained );

    ASSERT_EQ( 0, ring_buffer_first_pos( &rb ) );
  }

  byte obtained;

  /* now appends some extra bytes */
  for( size_t i = 0; i < rb_size * 2; i++ )
  {
    ring_buffer_append( &rb, rb_size + i );

    /* the oldest bytes is no longer available */
    ASSERT_EQ( i + 1, ring_buffer_first_pos( &rb ) );
    ASSERT_FALSE( ring_buffer_get( &rb, &obtained, i ) );

    /* but the next one is still the same */
    ASSERT_TRUE( ring_buffer_get( &rb, &obtained, i + 1 ) );
    ASSERT_EQ( i + 1, obtained );

    /* gets the new byte */
    ASSERT_TRUE( ring_buffer_get( &rb, &obtained, rb_size + i ) );
    ASSERT_EQ( rb_size + i, obtained );
  }

  ring_buffer_release( &rb );
}


TEST( ExtraTests )
{
  ring_buffer_t rb;
  size_t rb_size = 16;

  const char input[] = "some stupid sexy and funny string.";
  const char *expected = input + ( strlen( input ) - rb_size );

  ASSERT_TRUE( ring_buffer_init( &rb, rb_size ) );

  /* fills the buffer */
  for( size_t i = 0; i < strlen( input ); i++ )
    ring_buffer_append( &rb, input[i] );

  /* gets the first position available */
  uint64_t start = ring_buffer_first_pos( &rb );

  /* gets the buffer */
  byte obtained[rb_size];
  for( size_t i = start; i < start + rb_size; i++ )
    ASSERT_TRUE( ring_buffer_get( &rb, obtained + ( i - start ), i ) );

  /* checks it's the last portion of the string */
  ASSERT_EQ( 0, memcmp( expected, obtained, strlen( expected ) ) );

  ring_buffer_release( &rb );
}
