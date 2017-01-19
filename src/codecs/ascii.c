#include <stdio.h>
#include "ascii.h"


/** ASCII codec's internal structure. */
typedef struct
{
  /** Callback that handles the outputted data. */
  codec_out_cb_t out_cb;

  /** Parameter passed to the output callback. */
  void *out_cb_ctx;

  /** Indicates whether some data has been encoded yet (used for the separators). */
  bool has_encoded_data;

} ascii_codec_t;


/**
 * Writes an encoded literal.
 * @param  codec The codec instance.
 * @param  c     Character to write.
 * @return       \c true on success, \c false otherwise.
 */
static bool _write_literal( codec_t *codec, unsigned char c )
{
  ascii_codec_t *ic = codec->_int_data;

  char output[32];
  if( snprintf( output, sizeof( output ), "0%c", c ) >= sizeof( output ) )
    return false;

  /* checks if the separator should be added */
  if( ic->has_encoded_data )
    if( !ic->out_cb( " ", 1, ic->out_cb_ctx ) )
      return false;

  ic->has_encoded_data = true;

  /* sends the encoded data to the through the output callback */
  return ic->out_cb( output, strlen( output ), ic->out_cb_ctx );
}


/**
 * Writes an encoded match.
 * @param  codec The codec instance.
 * @param  m     The match to write.
 * @return       \c true on success, \c false otherwise.
 */
static bool _write_match( codec_t *codec, match_t m )
{
  ascii_codec_t *ic = codec->_int_data;

  char output[32];

  /* writes a pair (position, length) */
  if( snprintf( output, sizeof( output ), "1(%zu,%zu)", m.pos, m.len ) >= sizeof( output ) )
    return false;

  /* checks if the separator should be added */
  if( ic->has_encoded_data )
    if( !ic->out_cb( " ", 1, ic->out_cb_ctx ) )
      return false;

  ic->has_encoded_data = true;

  /* sends the encoded data to the through the output callback */
  return ic->out_cb( output, strlen( output ), ic->out_cb_ctx );
}


/**
 * Decodes and reads a character.
 * @param  codec The codec instance.
 * @param  c     The output character.
 * @return       \c true on success, \c false otherwise.
 */
static bool _read( codec_t *codec, unsigned char *c )
{
  /* TODO: implement */
  return false;
}


/**
 * Finishes the encoded output.
 * @param  codec The codec instance.
 * @return       \c true on success, \c false otherwise.
 */
static bool _close( codec_t *codec )
{
  ascii_codec_t *ic = codec->_int_data;

  const char output[] = "\n";

  /* encodes the new line + terminating null character */
  return ic->out_cb( output, sizeof( output ), ic->out_cb_ctx );
}


/**
 * Destroys the codec releasing all the taken resources.
 * @param codec Codec to destroy.
 */
static void _destroy( codec_t *codec )
{
  free( codec );
}


/**
 * Creates a new ASCII codec.
 * @param  cb            Callback used to output data.
 * @param  cb_ctx        Context passed to \a cb.
 * @param  min_match_len Minimum match length.
 * @param  max_match_len Maximum match length.
 * @param  max_pos       Maximum match position.
 * @return               Codec or \c NULL on error.
 */
codec_t *ascii_codec_create( codec_out_cb_t cb,
                             void *cb_ctx,
                             size_t min_match_len,
                             size_t max_match_len,
                             size_t max_pos )
{
  /* initializes the codec and returns it */
  NEW_CODEC( ascii_codec_t );

  ic->out_cb = cb;
  ic->out_cb_ctx = cb_ctx;

  return codec;
}
