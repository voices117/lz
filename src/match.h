#ifndef MATCH_H
#define MATCH_H


/* include area */
#include <stdbool.h>
#include <stdlib.h>


/** Patter match type. */
typedef struct
{
  /** Index of the position in the window. */
  size_t pos;

  /** Length of the match. */
  size_t len;

} match_t;


/** List of matches. */
typedef struct
{
  /** Number of matches in the list. */
  size_t num_elems;

  /** Number of elements in the list. */
  size_t list_size;

  /** A list of pointers to each match in the list. */
  match_t **match_idx;

  /** The actual list of matches. */
  match_t *matches;

} match_list_t;


/** Update callback type.
 *  The callback is called for every match element in the list.
 *  If the return value id \c false, the element is removed from the list.
 *  Otherwise, nothing is done with that element. */
typedef bool ( *ml_update_cb_t )( match_t *m, void *cb_ctx );


/** Prototypes. */
bool match_list_init( match_list_t *ml, size_t size );
void match_list_uninit( match_list_t *ml );

bool match_list_append( match_list_t *ml, const match_t *m );
size_t match_list_update( match_list_t *ml, ml_update_cb_t cb, void *cb_ctx );
bool match_list_get( const match_list_t *ml, size_t pos, match_t *m );


#endif
