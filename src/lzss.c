/* include area */
#include "lzss.h"


/* internal types */

/** Match list update context. */
typedef struct
{
  /** Window that buffered the previous bytes. */
  const window_t *w;

  /** New character to match against. */
  byte c;

} ml_update_cb_ctx_t;


static bool _find_match( size_t from, size_t *pos, const window_t *window, char c )
{
  for( size_t i = from; i < window_get_size( window ); i++ )
  {
    char wchar;
    if( !window_read( window, &wchar, i ) )
      /* TODO: inform better about the error */
      exit(1);

    if( wchar == c )
    {
      *pos = i;
      return true;
    }
  }

  return false;
}


static size_t _find_matches( const window_t *w, match_list_t *ml, char c )
{
  match_t m;
  size_t wpos = 0, found_matches = 0;
  while( _find_match( wpos, &wpos, w, c ) )
  {
    m.pos = wpos;
    m.len = 1;

    /* adds a match into the list */
    if( !match_list_append( ml, &m ) )
      /* TODO: inform better about the error */
      exit(2);

    found_matches++;
    wpos++;
  }

  return found_matches;
}


static bool _ml_update_cb( match_t *m, void *cb_ctx )
{
  ml_update_cb_ctx_t *ctx = cb_ctx;

  char wchar;
  if( !window_read( ctx->w, &wchar, m->pos ) )
    /* TODO: inform better about the error */
    exit( 5 );

  /* if it doesn't match anymore, removes the match from the list */
  if( wchar != ctx->c )
    return false;

  /* still matching at that position, increments the match length */
  m->len += 1;

  return true;
}


static size_t _update_matches( match_list_t *ml, const window_t *w, byte c, match_t *best_match )
{
  ml_update_cb_ctx_t cb_ctx = {
    .c = c,
    .w = w
  };

  /* gets the first match in the list, in case there are no more matches left in the window */
  if( !match_list_get( ml, 0, best_match ) )
    /* TODO: inform better about the error */
    exit( 6 );

  /* updates the list removing all matches that are not valid anymore */
  return match_list_update( ml, _ml_update_cb, &cb_ctx );
}


static lzss_error_t _compress_one( lzss_t *lz, byte b )
{
  /* if there are already matches in the window, updates them */
  if( match_list_length( &lz->ml ) > 0 )
  {
    match_t match;

    /* updates matches */
    if( _update_matches( &lz->ml, &lz->window, b, &match ) == 0 )
    {
      /* no more matches
       * if there was a valid match, it's written or else all matched bytes are outputted as
       * literals */
      if( match.len >= lz->min_match_len )
      {
        if( !lz->codec->write_match( lz->codec, match ) )
          return lzss_error_io_error;

        lz->current_match_len = 0;
      }
      else
      {
        /* outputs all buffered bytes as literals */
        for( size_t i = 0; i < match.len; i++ )
          if( !lz->codec->write_literal( lz->codec, lz->current_match[i] ) )
            return lzss_error_io_error;

        lz->current_match_len = 0;
      }
    }
    else if( match.len < lz->min_match_len )
      lz->current_match[lz->current_match_len++] = b;
  }

  if( match_list_length( &lz->ml ) == 0 &&
      _find_matches( &lz->window, &lz->ml, b ) > 0 &&
      ( lz->current_match_len < lz->min_match_len ) )
    lz->current_match[lz->current_match_len++] = b;
  else if( match_list_length( &lz->ml ) == 0 && !lz->codec->write_literal( lz->codec, b ) )
    return lzss_error_io_error;

  if( match_list_length( &lz->ml ) > 0 )
  {
    match_t match;
    /* gets the first match in the list, in case there are no more matches left in the window */
    if( !match_list_get( &lz->ml, 0, &match ) )
      /* TODO: inform better about the error */
      exit( 6 );

    if( match.len == lz->max_match_len )
    {
      if( !lz->codec->write_match( lz->codec, match ) )
        return lzss_error_io_error;

      lz->current_match_len = 0;
      match_list_reset( &lz->ml );
    }
  }

  /* appends the new byte into the window */
  window_append( &lz->window, b );

  return lzss_error_no_error;
}


/**
 * Initializes the LZSS to compress/decompress data.
 * @param  lz            LZSS to initialize.
 * @param  window_size   Size of the window to use.
 * @param  min_match_len Minimum size of bytes required to be encoded as match.
 * @param  max_match_len Maximum bytes allowed in a match.
 * @param  codec         Codec used to encode/decode the data.
 * @return               Error code.
 */
lzss_error_t lzss_init( lzss_t *lz,
                        size_t window_size,
                        size_t min_match_len,
                        size_t max_match_len,
                        codec_t *codec )
{
  /* initializes the internal window buffer */
  if( !window_init( &lz->window, window_size ) )
    return lzss_error_malloc_error;

  /* initializes the internal match buffer.
   * this buffer holds the characters that are currently matching, but have not yet got to the
   * minimum match length, so the may end up being encoded as literals */
  lz->current_match_len = 0;
  lz->current_match = malloc( min_match_len );
  if( lz->current_match == NULL )
    goto error0;

  if( !match_list_init( &lz->ml, window_size ) )
    goto error1;

  /* sets the other parameters */
  lz->codec = codec;
  lz->min_match_len = min_match_len;
  lz->max_match_len = max_match_len;
  lz->state = lzss_state_init;

  return lzss_error_no_error;

error1:
  free( lz->current_match );

error0:
  window_release( &lz->window );

  return lzss_error_malloc_error;
}


/**
 * Compress some data.
 * This function can be called many times to compress by chunks.
 * @param  lz   An already initialized LZSS.
 * @param  data Data to be compressed.
 * @param  size Size of \a data.
 * @return      Error code.
 */
lzss_error_t lzss_compress( lzss_t *lz, const void *data, size_t size )
{
  const byte *bytes = data;
  lzss_error_t error;

  /* compresses byte by byte */
  for( size_t i = 0; i < size; i++ )
  {
    error = _compress_one( lz, bytes[i] );
    if( error != lzss_error_no_error )
      return error;
  }

  /* success */
  return lzss_error_no_error;
}


/**
 * Ends the compression/decompression.
 * @param  lz An already initialized and fed LZSS.
 * @return    Error code.
 */
lzss_error_t lzss_end( lzss_t *lz )
{
  /* checks if there's data left to be written */
  if( match_list_length( &lz->ml ) > 0 )
  {
    match_t match;

    /* gets the first match (any match will do since they share the length) */
    if( !match_list_get( &lz->ml, 0, &match ) )
      return lz_error_internal_error;

    if( match.len >= lz->min_match_len )
    {
      if( !lz->codec->write_match( lz->codec, match ) )
        return lzss_error_io_error;
    }
    else
    {
      for( size_t i = 0; i < match.len; i++ )
        if( !lz->codec->write_literal( lz->codec, lz->current_match[i] ) )
          return lzss_error_io_error;

      lz->current_match_len = 0;
    }
  }

  /* informs the codec there's no more data left to be processed */
  if( !lz->codec->close( lz->codec ) )
    return lzss_error_io_error;

  /* success */
  return lzss_error_no_error;
}


/**
 * Releases all resources.
 * The LZSS cannot be used unless initialized again.
 * @param lz An initialized LZSS.
 */
void lzss_uninit( lzss_t *lz )
{
  match_list_uninit( &lz->ml );
  window_release( &lz->window );
  free( lz->current_match );
  lz->codec = NULL;
}
