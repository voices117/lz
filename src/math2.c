#include "math2.h"


/**
 * Calculates the base 2 log of \a n.
 * @param  n Number who's base 2 logarithm is calculated.
 * @return   Base 2 logarithm.
 */
double log2( double n )
{
  return log( n ) / log( 2 );
}


size_t math_bits_in_n( size_t n )
{
  size_t num_bits = 0;
  while( ( n >> num_bits ) > 0 )
    num_bits += 1;

  return num_bits;
}
