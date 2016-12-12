#ifndef CODEC_H
#define CODEC_H


/* include area */
#include <stdbool.h>
#include "../window.h"
#include "../match.h"


/** Codec implementation initializer. */
#define CODEC_INIT()  ( codec_t ) {\
    .write_literal = _write_literal,\
    .write_match = _write_match,\
    .read = _read,\
    .close = _close,\
    .destroy = _destroy,\
    ._int_data = NULL\
  }


/** Codec forward declaration. */
typedef struct codec codec_t;


/** Codec API. */
struct codec
{
  /** Codec's internal data (implementation specific, internal use only!!!). */
  void *_int_data;

  /** Writes an encoded literal character. */
  bool ( *write_literal )( codec_t *codec, char c );

  /** Writes an encoded match. */
  bool ( *write_match )( codec_t *codec, match_t m );

  /** Reads a character from the codec. */
  bool ( *read )( codec_t *codec, char *c );

  /** Closes the codec (finishes the encoding, like flushing pending data). */
  bool ( *close )( codec_t *codec );

  /** Destroys the codec. */
  void ( *destroy )( codec_t *codec );

};


#endif
