#include "booksim.hpp"
#include <stdlib.h>

#include "random_utils.hpp"

void RandomSeed( long seed )
{
  srand48( seed );
  //int_sgenrand( seed );
  //float_sgenrand( seed );
}

int RandomInt( int max ) 
  // Returns a random integer in the range [0,max]
{
  return ( lrand48( ) % (max+1) );
  //return ( int_genrand( ) % (max+1) );
}

unsigned long RandomIntLong( )
{
  //return int_genrand( );
  return lrand48( );
}

float RandomFloat( float max )
  // Returns a random floating-point value in the rage [0,max]
{
  return ( drand48( ) * max );
  //return float_genrand( ) * max;
}
