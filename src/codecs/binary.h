#ifndef BINARY_H
#define BINARY_H


/* include area */
#include "codec.h"


/* prototypes */
codec_t *binary_codec_create( codec_out_cb_t cb,
                              void *cb_ctx,
                              size_t min_match_len,
                              size_t max_match_len,
                              size_t max_pos );


#endif
