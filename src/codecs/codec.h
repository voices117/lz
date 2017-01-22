#ifndef CODEC_H
#define CODEC_H


/* include area */
#include <stdbool.h>
#include <string.h>
#include "../match.h"
#include "../datatype.h"


/** Codec implementation initializer. */
#define CODEC_INIT( int_data )  ( codec_t ) {  \
    .write_literal = _write_literal,           \
    .write_match = _write_match,               \
    .read = _read,                             \
    .close = _close,                           \
    .destroy = _destroy,                       \
    ._int_data = int_data                      \
  }


/** Allocates the required memory to create a codec with an internal data of type \a int_codec_type.
 *  This macro creates 2 variables:
 *    \a codec, which is a pointer to the codec API and the value that should be returned.
 *    \a ic, which is a pointer to the internal codec implementation to be initialized. */
#define NEW_CODEC( int_codec_type )             \
  /* struct containing both the API and codec */\
  struct codec                                  \
  {                                             \
    codec_t api;                                \
    int_codec_type ic;                          \
  } *buf = calloc( 1, sizeof( struct codec ) ); \
  if( !buf ) return NULL;                       \
                                                \
  /* sets the internal codec pointer */         \
  int_codec_type *ic = &buf->ic;                \
                                                \
  /* creates and initializes the API pointer */ \
  codec_t *codec = &buf->api;                   \
  buf->api = CODEC_INIT( ic )


/** Callback used by the codec to output the encoded data.
 *  Receives the data to output, the size of the data and the user provided context. */
typedef bool ( *codec_out_cb_t )( const void *buffer, size_t buffer_size, void *ctx );


/** Codec type forward declaration. */
typedef struct codec codec_t;


/** Codec API. */
struct codec
{
  /** Codec's internal data (implementation specific, internal use only!!!). */
  void *_int_data;

  /** Writes an encoded literal character. */
  bool ( *write_literal )( codec_t *codec, byte c );

  /** Writes an encoded match. */
  bool ( *write_match )( codec_t *codec, match_t m );

  /** Reads a character from the codec. */
  bool ( *read )( codec_t *codec, byte *c );

  /** Closes the codec (finishes the encoding, like flushing pending data). */
  bool ( *close )( codec_t *codec );

  /** Destroys the codec. */
  void ( *destroy )( codec_t *codec );

};


#endif
