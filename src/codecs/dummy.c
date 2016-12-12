#include "dummy.h"


/**
 * Writes an encoded literal.
 * @param  codec The codec instance.
 * @param  c     Character to write.
 * @return       \c true on success, \c false otherwise.
 */
static bool _write_literal( codec_t *codec, char c )
{
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
  return true;
}


/**
 * Destroys the codec releasing all the taken resources.
 * @param codec Codec to destroy.
 */
static void _destroy( codec_t *codec )
{
  /* does nothing */
}


/**
 * Creates a new ASCII codec.
 * @return  ASCII codec or \c NULL in case of error.
 */
codec_t dummy_codec_create( void )
{
  /* initializes the codec and returns it */
  codec_t c = CODEC_INIT();
  return c;
}

