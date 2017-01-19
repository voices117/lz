#ifndef MATH2_H
#define MATH2_H


/* includes */
#include <math.h>
#include <stdio.h>


/* macros */
#define MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define MIN( a, b ) ( ( a ) < ( b ) ? ( a ) : ( b ) )


/** prototypes  */
double log2( double n );
size_t math_bits_in_n( size_t n );

#endif
