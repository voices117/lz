#ifndef LZSS_H
#define LZSS_H


/* include area */
#include <stdlib.h>
#include "codecs/codec.h"
#include "window.h"
#include "match.h"


/* data types */

/** Library error codes. */
typedef enum
{
  /** No error in the operation. */
  lzss_error_no_error,

  /** Failed allocating the required memory. */
  lzss_error_malloc_error,

  /** Failed reading/writing through a codec. */
  lzss_error_io_error,

  /** Something unexpected went wrong (probably a programmer's error). */
  lz_error_internal_error,

} lzss_error_t;


/** Internal state. */
typedef enum
{
  /** First stage of the algorithm. */
  lzss_state_init,

} lzss_state_t;


/** LZSS  data type. */
typedef struct
{
  /** Codec used to encode/decode data. */
  codec_t *codec;

  /** Minimum match length. */
  size_t min_match_len;

  /** Maximum match length. */
  size_t max_match_len;

  /** Window buffer. */
  window_t window;

  /** Window to store the current matched bytes (but not yet outputted). */
  window_t current_match;

  /** List of window matches. */
  match_list_t ml;

  /** Current state. */
  lzss_state_t state;

} lzss_t;


/* function prototypes */
lzss_error_t lzss_init( lzss_t *lz,
                        size_t window_size,
                        size_t min_match_len,
                        size_t max_match_len,
                        codec_t *codec );
lzss_error_t lzss_compress( lzss_t *lz, const void *data, size_t size );
lzss_error_t lzss_end( lzss_t *lz );
void lzss_uninit( lzss_t *lz );


#endif
