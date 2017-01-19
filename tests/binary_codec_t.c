#include <string.h>
#include "scunit.h"
#include "codecs/binary.h"


#define ASIZE( array )  ( sizeof( array ) / sizeof( array[0] ) )


/* encodes the data in \a data and checks it's equal to the data in \a expected.
 * both input values must be arrays. */
#define ENCODE_AND_CHECK( data, expected )                             \
  do {                                                                 \
    /* buffer to store the encoded data */                             \
    struct buffer obtained = { { 0 } };                                \
                                                                       \
    /* creates the encoder */                                          \
    codec_t *bc = binary_codec_create( _out_cb, &obtained, 2, 2, 2 );  \
    ASSERT_NE( NULL, bc );                                             \
                                                                       \
    /* fills the data to the encoder */                                \
    for( size_t i = 0; i < ASIZE( data ); i++ )                        \
        ASSERT_TRUE( bc->write_literal( bc, data[i] ) );               \
                                                                       \
    /* ends the encoder */                                             \
    ASSERT_TRUE( bc->close( bc ) );                                    \
                                                                       \
    /* checks the encoded data is the same as the expected data */     \
    ASSERT_EQ( obtained.size, ASIZE( expected ) );                     \
    ASSERT_EQ( memcmp( expected, obtained.b, sizeof( expected ) ), 0 );\
                                                                       \
    /* releases resources */                                           \
    bc->destroy( bc );                                                 \
  } while (0)


/* buffer with size struct */
struct buffer
{
  byte b[1024];

  size_t size;
};


/**
 * Output callback used to test the binary codec.
 * @param  buffer Encoded data.
 * @param  size   \a buffer size.
 * @param  ctx    Pointer where the decoded data is stored.
 * @return        Always \c true.
 */
static bool _out_cb( const void *buffer, size_t size, void *ctx )
{
  struct buffer *output = ctx;

  /* sets the encoded byte in the current position */
  memcpy( output->b + output->size, buffer, size );

  /* point to the next index */
  output->size += size;

  return true;
}


TEST( EmptyOutput )
{
  /* buffer to store the encoded data */
  struct buffer obtained = { { 0 } };

  /* expects just the padding */
  byte expected[] = { 0x80 };

  /* creates the encoder */
  codec_t *bc = binary_codec_create( _out_cb, &obtained, 2, 2, 2 );
  ASSERT_NE( NULL, bc );

  /* does not encode any data */
  ASSERT_TRUE( bc->close( bc ) );

  /* checks the encoded data is the same as the expected data */
  ASSERT_EQ( obtained.size, ASIZE( expected ) );
  ASSERT_EQ( memcmp( expected, obtained.b, ASIZE( expected ) ), 0 );

  /* releases resources */
  bc->destroy( bc );
}


TEST( Literals )
{
  /* basic test */
  {
    byte data[] = { 0x55, 0x00, 0xff, 0x33 };

    /* append a 0 before each byte and finish with the padding */
    byte expected[] = { 0x2a, 0x80, 0x1f, 0xe3, 0x38 };

    ENCODE_AND_CHECK( data, expected );
  }

  /* full byte overhead */
  {
    byte data[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    /* every byte has a 1 bit overhead, so in the end a full extra byte is added to the encoded data
     * there's also a full extra byte for padding */
    byte expected[] = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe, 0xff, 0x80 };

    ENCODE_AND_CHECK( data, expected );
  }
}


TEST( Matches )
{
  /* basic test */
  {
    /* buffer to store the encoded data */
    struct buffer obtained = { { 0 } };

    /* 1 bit as match prefix, 10 zero bits for the position and 4 bits for the match length plus
       1 bits padding.
       the match length is the min length, so it's encoded as zero */
    byte expected[] = { 0x80, 0x01 };

    match_t m = {
      .pos = 0,
      .len = 2
    };

    codec_t *bc = binary_codec_create( _out_cb, &obtained, 2, 10, 1024 );
    ASSERT_NE( NULL, bc );

    bc->write_match( bc, m );
    bc->close( bc );

    ASSERT_EQ( obtained.size, ASIZE( expected ) );
    ASSERT_EQ( memcmp( obtained.b, expected, sizeof( expected ) ), 0 );

    bc->destroy( bc );
  }
}
