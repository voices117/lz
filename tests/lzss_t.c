#include <string.h>
#include "codecs/ascii.h"
#include "lzss.h"
#include "scunit.h"


/** Asserts that the returned value is not an lzs_* error. */
#define ASSERT_NO_ERROR( expression )  ASSERT_EQ( lzss_error_no_error, ( expression ) )

/** Asserts that an obtained buffer contains the expected string.
 *  If it doesn't, the string is printed */
#define ASSERT_COMPRESSED( expected, obtained )                      \
  do {                                                               \
    if( sizeof( expected ) != obtained.data_len ||                   \
        memcmp( expected, obtained.data, sizeof( expected ) ) != 0 ) \
    {                                                                \
      printf( "expected: %s", expected );                            \
      printf( "obtained: %s", obtained.data );                       \
      ASSERT_FALSE( true );                                          \
    }                                                                \
  }                                                                  \
  while(0)

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
    ASSERT_COMPRESSED( expected, obtained );                                       \
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
     * match (at that time, the 'a' will be at pos 0) */
    const char expected[] = "0b 1(0,4) 0a 1(0,9)\n";

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
    const char expected[] = "0a 0a 0a 0a 0a 0a 0a 0a 0b 0b 0b 0b 0b 0b 0b 0b 1(15,8) 0a 0a 0a\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }
}


TEST( MaxMatchLength )
{
  {
    #define WINDOW_SIZE 256
    #define MIN_MATCH 8
    #define MAX_MATCH 15

    const char data[] = "aaaaaaaaaaaaaaaaaa";
    const char expected[] = "0a 1(0,15) 0a 0a\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 256
    #define MIN_MATCH 3
    #define MAX_MATCH 15

    const char data[] = "123456789123456789123456789";
    const char expected[] = "01 02 03 04 05 06 07 08 09 1(8,15) 1(8,3)\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }
}


TEST( ComplexStrings )
{
  {
    #define WINDOW_SIZE 1024
    #define MIN_MATCH 8
    #define MAX_MATCH 1024

    const char data[] = "abcabcabcabcabcabc";
    const char expected[] = "0a 0b 0c 1(2,15)\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 1024
    #define MIN_MATCH 8
    #define MAX_MATCH 1024

    const char data[] = "abbbbcabcabcabcabcabd";
    const char expected[] = "0a 0b 0b 0b 0b 0c 0a 0b 1(2,12) 0d\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 1024
    #define MIN_MATCH 4
    #define MAX_MATCH 1024

    const char data[] = "abcd 1 2 3 4 5 6 abcdaa";
    const char expected[] = "0a 0b 0c 0d 0  01 0  02 0  03 0  04 0  05 0  06 0  1(16,4) 0a 0a\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 4
    #define MIN_MATCH 2
    #define MAX_MATCH 1024

    const char data[] = "ABCDADADAABDAA";
    const char expected[] = "0A 0B 0C 0D 0A 1(1,4) 0A 0B 1(3,3)\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 4
    #define MIN_MATCH 2
    #define MAX_MATCH 1024

    const char data[] = "ABCDADADAABCAA";
    const char expected[] = "0A 0B 0C 0D 0A 1(1,4) 0A 0B 0C 1(3,2)\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }
}


TEST( ShortWindow )
{
  {
    #define WINDOW_SIZE 16
    #define MIN_MATCH 4
    #define MAX_MATCH 1024

    const char data[] = "aaaaaaaaaaaaaaaaaaaa";
    const char expected[] = "0a 1(0,19)\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 64
    #define MIN_MATCH 4
    #define MAX_MATCH 1024

    const char data[] = "there are 10 types of people in the world: those who understand binary, "\
                        "and those who don't.";
    const char expected[] = "0t 0h 0e 0r 0e 0  0a 0r 0e 0  01 00 0  0t 0y 0p 0e 0s 0  0o 0f 0  0p 0e"\
                            " 0o 0p 0l 0e 0  0i 0n 0  0t 0h 0e 0  0w 0o 0r 0l 0d 0: 0  0t 0h 0o 0s 0e"
                            " 0  0w 0h 0o 0  0u 0n 0d 0e 0r 0s 0t 0a 0n 0d 0  0b 0i 0n 0a 0r 0y 0, 0 "
                            " 1(11,4) 1(32,10) 0d 0o 0n 0' 0t 0.\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }

  {
    #define WINDOW_SIZE 32
    #define MIN_MATCH 4
    #define MAX_MATCH 1024

    const char data[] = "six sick hicks nick six slick bricks with picks and sticks.";
    const char expected[] = "0s 0i 0x 0  0s 0i 0c 0k 0  0h 0i 0c 0k 0s 0  0n 1(10,4) 1(19,5) 0l "
                            "1(20,4) 0b 0r 1(21,5) 0w 0i 0t 0h 0  0p 1(10,5) 0a 0n 0d 0  0s 0t "
                            "1(10,4) 0.\n";

    TEST_W_ASCII( expected, data, WINDOW_SIZE, MIN_MATCH, MAX_MATCH );

    #undef WINDOW_SIZE
    #undef MIN_MATCH
    #undef MAX_MATCH
  }
}
