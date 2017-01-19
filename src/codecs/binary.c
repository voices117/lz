/* include area */
#include "binary.h"
#include "../math2.h"
#include "stdio.h"


/** Constants */
#define BITS_IN_BYTE 8U

/** Number of bits in the output buffer of the binary codec. */
#define BITS_IN_BC_BUFFER BITS_IN_BYTE


/** Macros */

/** Appends the \a num_bits least significant bits of \a c to the output buffer in the binary codec
 *  \a bc. If the output buffer gets full, the content is flushed. */
#define APPEND_BITS( bc, c, num_bits )\
  do {\
    size_t bytes_free = ( BITS_IN_BC_BUFFER - bc->bits_stored );\
    if( bytes_free > ( num_bits ) )\
    {\
      ( bc )->output |= ( ( c ) << ( bytes_free - ( num_bits ) ) );\
      ( bc )->bits_stored += ( num_bits );\
    }\
    else\
    {\
      unsigned char output = ( bc )->output | ( ( c ) >> ( ( num_bits ) - bytes_free ) );\
      if( !( bc )->out_cb( &output, sizeof( output ), ( bc )->out_cb_ctx ) )\
        return false;\
      ( bc )->output = ( c ) << ( BITS_IN_BC_BUFFER - ( ( num_bits ) - bytes_free ) );\
      ( bc )->bits_stored = ( num_bits ) - bytes_free;\
    }\
  } while( 0 )


/** Data types */

/** Binary codec internal data. */
typedef struct
{
  /** Holds the (partial) output of the codec. */
  unsigned int output;

  /** Number of bits (most significant) already set in output from previous writes. */
  size_t bits_stored;

  /** Callback to output encoded data. */
  codec_out_cb_t out_cb;

  /** Context passed to the output callback. */
  void *out_cb_ctx;

  /** Minimum match length. */
  size_t min_match_len;

  /** Number of bits required to represent an encoded match. */
  size_t num_bits_match;

  /** Maximum match position. */
  size_t num_bits_pos;

} binary_codec_t;


/**
 * Writes an encoded literal.
 * @param  codec The codec instance.
 * @param  c     Character to write.
 * @return       \c true on success, \c false otherwise.
 */
static bool _write_literal( codec_t *codec, unsigned char c )
{
  binary_codec_t *bc = codec->_int_data;

  /* a literal is made of a zero bit and the byte literal */
  APPEND_BITS( bc, 0x00, 1 );
  APPEND_BITS( bc, c, BITS_IN_BYTE );

  /* success */
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

  /* sets the bit indicating the match */
  APPEND_BITS( bc, 0x01, 1 );

  /* encodes the position */
  APPEND_BITS( bc, m.pos, bc->num_bits_pos );

  /* encodes the match length */
  APPEND_BITS( bc, m.len - bc->min_match_len, bc->num_bits_match );

  /* success */
  return true;
}


/**
 * Decodes and reads a character.
 * @param  codec The codec instance.
 * @param  c     The output character.
 * @return       \c true on success, \c false otherwise.
 */
static bool _read( codec_t *codec, unsigned char *c )
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
  binary_codec_t *bc = codec->_int_data;

  unsigned char output;

  if( bc->bits_stored > 0 )
  {
    /* outputs the remaining bits and the padding */
    output = bc->output | ( 1U << ( BITS_IN_BC_BUFFER - bc->bits_stored - 1 ) );
  }
  else
  {
    /* outputs a full byte of padding */
    output = 0x80;
  }

  return bc->out_cb( &output, sizeof( output ), bc->out_cb_ctx );
}


/**
 * Destroys the codec releasing all the taken resources.
 * @param codec Codec to destroy.
 */
static void _destroy( codec_t *codec )
{
  /* frees the allocated memory */
  free( codec );
}


/**
 * Creates a new binary codec.
 * @param  cb            Callback used to output data.
 * @param  cb_ctx        Context passed to \a cb.
 * @param  min_match_len Minimum match length.
 * @param  max_match_len Maximum match length.
 * @param  max_pos       Maximum match position.
 * @return               Codec or \c NULL on error.
 */
codec_t *binary_codec_create( codec_out_cb_t cb,
                              void *cb_ctx,
                              size_t min_match_len,
                              size_t max_match_len,
                              size_t max_pos )
{
  /* input checks */
  if( max_match_len < 2 || min_match_len < 2 )
    return NULL;
  if( min_match_len > max_match_len )
    return NULL;
  if( max_pos < 2 )
    return NULL;

  NEW_CODEC( binary_codec_t );

  /* initializes the internal binary codec */
  ic->out_cb = cb;
  ic->out_cb_ctx = cb_ctx;
  ic->min_match_len = min_match_len;

  /* calculates the number of bits required to encode the match length */
  ic->num_bits_match = math_bits_in_n( ( max_match_len - min_match_len ) + 1 );
  ic->num_bits_pos = math_bits_in_n( max_pos - 1 );

  return codec;
}
