/* include area */
#include "binary.h"
#include "stdio.h"


/** Binary codec internal data. */
typedef struct
{
  /** Holds the (partial) output of the codec. */
  int output;

  /** Number of bits (most significant) already set in output. */
  size_t bits_set;

} binary_codec_t;


/**
 * Writes an encoded literal.
 * @param  codec The codec instance.
 * @param  c     Character to write.
 * @return       \c true on success, \c false otherwise.
 */
static bool _write_literal( codec_t *codec, char c )
{
  binary_codec_t *bc = codec->_int_data;

  /* a literal is made of a zero bit and the byte, so it needs an extra bit */
  char output = bc->output & ( c >> ( bc->bits_set + 1 ) );

  /* sets the number of bytes left to be written */
  bc->bits_set = ( 8 + 1 ) - bc->bits_set;

  /* sets the unwritten bits in the output buffer */
  bc->output = ( c << bc->bits_set );

  for( size_t i = 0; i < 8; i++ )
    printf( "%c", ( output & ( 0x80 >> i ) ) ? '1' : '0' );
  printf( " " );

  return true;
}


/**
 * Writes an encoded match.
 * @param  codec The codec instance.
 * @param  m     The match to write.
 * @return       \c true on success, \c false otherwise.
 */
static bool _write_match( codec_t *codec, match_t m )
{
  binary_codec_t *bc = codec->_int_data;

  /* a match is made of a 1 bit and then the position and length */
  char output = bc->output & ( 8 >> ( bc->bits_set + 1 ) );
  (void)output;

  /* writes a pair (position, length) */
  printf( "1(%zu,%zu)", m.pos, m.len );
  return true;
}


/**
 * Decodes and reads a character.
 * @param  codec The codec instance.
 * @param  c     The output character.
 * @return       \c true on success, \c false otherwise.
 */
static bool _read( codec_t *codec, char *c )
{
  return false;
}


/**
 * Finishes the encoded output.
 * @param  codec The codec instance.
 * @return       \c true on success, \c false otherwise.
 */
static bool _close( codec_t *codec )
{
  /* outputs the remaining bits and the padding */
  printf( "\n" );
  return true;
}


/**
 * Destroys the codec releasing all the taken resources.
 * @param codec Codec to destroy.
 */
static void _destroy( codec_t *codec )
{
  /* frees the allocated memory */
  free( codec->_int_data );
}


/**
 * Creates a new binary codec.
 * @return  Binary codec.
 */
codec_t binary_create_codec( void )
{
  /* sets the codec's API */
  codec_t codec = CODEC_INIT();

  /* sets the codec's internal data */
  codec._int_data = calloc( 1, sizeof( binary_codec_t ) );

  return codec;
}
