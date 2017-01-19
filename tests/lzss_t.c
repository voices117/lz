#include <string.h>
#include "codecs/ascii.h"
#include "lzss.h"
#include "scunit.h"


/** Asserts that the returned value is not an lzs_* error. */
#define ASSERT_NO_ERROR( expression )  ASSERT_EQ( lzss_error_no_error, ( expression ) )


/* buffer with size */
struct buffer
{
  char data[2048];
  size_t data_len;
};


/**
 * Callback that stores the encoded data.
 * @param  data Encoded data.
 * @param  size Encoded data size.
 * @param  ctx  Output string.
 * @return      \c true on success, \c false otherwise.
 */
static bool _ascii_codec_out_cb( const void *data, size_t size, void *ctx )
{
  struct buffer *b = ctx;

  if( b->data_len + size > sizeof( b->data ) )
    return false;

  memcpy( b->data + b->data_len, data, size );
  b->data_len += size;
  return true;
}


/**
 * Feeds \a input into an LZSS with an ASCII codec and asserts the output is exactly the same as
 * \a expected.
 * @param  expected Expected output.
 * @param  input    String to compress.
 */
#define TEST_W_ASCII( expected, input, min_match_len, max_match_len, window_size ) \
  do {                                                                             \
    /* encoded data */                                                             \
    struct buffer obtained;                                                        \
    memset( &obtained, 0, sizeof( obtained ) );                                    \
                                                                                   \
    /* creates the output encoder */                                               \
    codec_t *codec = ascii_codec_create( _ascii_codec_out_cb,                      \
                                         &obtained,                                \
                                         min_match_len,                            \
                                         max_match_len,                            \
                                         window_size );                            \
                                                                                   \
    lzss_t lz;                                                                     \
                                                                                   \
    /* compresses the data */                                                      \
    ASSERT_NO_ERROR( lzss_init( &lz, WINDOW_SIZE, MIN_MATCH, MAX_MATCH, codec ) ); \
    ASSERT_NO_ERROR( lzss_compress( &lz, data, strlen( data ) ) );                 \
    ASSERT_NO_ERROR( lzss_end( &lz ) );                                            \
                                                                                   \
    ASSERT_EQ( sizeof( expected ), obtained.data_len );                            \
    ASSERT_EQ( 0, memcmp( expected, obtained.data, sizeof( expected ) ) );         \
                                                                                   \
    /* releases memory */                                                          \
    codec->destroy( codec );                                                       \
    lzss_uninit( &lz );                                                            \
  } while(0)


TEST( BasicCompression )
{
  /* basic compression test */
  {
    #define WINDOW_SIZE 10
    #define MIN_MATCH 4
    #define MAX_MATCH 1024

    const char data[] = "aaaaaaaaaa";

    /* expects a literal 'a' followed by a match from pos 0 of len 9 */
    const char expected[] = "0a 1(0,9)\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 10
    #define MIN_MATCH 4
    #define MAX_MATCH 1024

    const char data[] = "bbbbbaaaaaaaaaa";

    /* expects a literal 'b' followed by a match from pos 0 of len 4, a literal 'a' and another
     * match */
    const char expected[] = "0b 1(0,4) 0a 1(5,9)\n";

   TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }
}


TEST( MinMatchLength )
{
  /* min length not reached */
  {
    #define WINDOW_SIZE 1024
    #define MIN_MATCH 10
    #define MAX_MATCH 1024

    const char data[] = "aaaaaaaaaa";

    /* expects 10 literal 'a' because the min match length is not reached */
    const char expected[] = "0a 0a 0a 0a 0a 0a 0a 0a 0a 0a\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );
  }

  /* min length reached exactly */
  {
    /* now adds an extra 'a' to the match length is valid */
    const char data[] = "aaaaaaaaaaa";

    /* expects 9 literal 'a' because the min match length was not reached */
    const char expected[] = "0a 1(0,10)\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 1024
    #define MIN_MATCH 8
    #define MAX_MATCH 1024

    const char data[] = "aaaaaaaabbbbbbbbaaaaaaaaaaa";

    /* expects 9 literal 'a' because the min match length was not reached */
    const char expected[] = "0a 0a 0a 0a 0a 0a 0a 0a 0b 0b 0b 0b 0b 0b 0b 0b 0a 1(16,10)\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }
}
