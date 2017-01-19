#ifndef ASCII_H
#define ASCII_H


/* include area */
#include "codec.h"


/* prototypes */
codec_t *ascii_codec_create( codec_out_cb_t cb,
                             void *cb_ctx,
                             size_t min_match_len,
                             size_t max_match_len,
                             size_t max_pos );


#endif
