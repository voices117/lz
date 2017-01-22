#include "match.h"


/** Gets the match from the list at a given (logical) position. */
#define MATCH_AT( ml, pos )  ( ( ml )->matches[ml->match_idx[pos]] )


bool match_list_init( match_list_t *ml, size_t size )
{
  ml->num_elems = 0;
  ml->list_size = size;

  /* initializes the array of pointers */
  ml->match_idx = calloc( size, sizeof( match_t * ) );
  if( ml->match_idx == NULL )
    return false;

  /* initializes the array of matches */
  ml->matches = calloc( size, sizeof( match_t ) );
  if( ml->match_idx == NULL )
    goto error0;

  return true;

error0:
  free( ml->match_idx );
  return false;
}


void match_list_uninit( match_list_t *ml )
{
  ml->list_size = 0;
  free( ml->match_idx );
  free( ml->matches );
}


bool match_list_append( match_list_t *ml, const match_t *m )
{
  /* checks if the list is full */
  if( ml->num_elems >= ml->list_size )
    return false;

  /* FIXME: assumes position not used!!! */
  match_t *e = &ml->matches[ml->num_elems];

  e->pos = m->pos;
  e->len = m->len;

  /* sets the pointer to the match */
  ml->match_idx[ml->num_elems] = e;

  ml->num_elems += 1;

  return true;
}


size_t match_list_update( match_list_t *ml, ml_update_cb_t cb, void *cb_ctx )
{
  size_t index = 0;
  while( ml->match_idx[index] != NULL )
  {
    if( !cb( ml->match_idx[index], cb_ctx ) )
    {
      /* reduces the number of elements held in the list */
      ml->num_elems -= 1;

      /* swaps the last element with the element in the current position */
      ml->match_idx[index] = ml->match_idx[ml->num_elems];

      /* deletes the last element */
      ml->match_idx[ml->num_elems] = NULL;

      /* reduces the index because the same position now contains a new element so it must be
       * checked again */
      index--;
    }

    index++;
  }

  return ml->num_elems;
}


bool match_list_get( const match_list_t *ml, size_t pos, match_t *m )
{
  if( pos >= ml->num_elems )
    return false;

  /* gets a pointer to the corresponding match instance */
  match_t *e = ml->match_idx[pos];

  /* clones the match into the output parameter */
  m->pos = e->pos;
  m->len = e->len;

  return true;
}


void match_list_reset( match_list_t *ml )
{
  ml->num_elems = 0;
}


size_t match_list_length( const match_list_t *ml )
{
  return ml->num_elems;
}
