#ifndef _RANDOM_UTILS_HPP_
#define _RANDOM_UTILS_HPP_

#ifdef USE_TWISTER
extern unsigned long  int_genrand( );
extern void int_lsgenrand( unsigned long seed_array[] );
extern void int_sgenrand( unsigned long seed );

extern double float_genrand( );
extern void   float_lsgenrand( unsigned long seed_array[] );
extern void   float_sgenrand( unsigned long seed );
#endif USE_TWISTER

void RandomSeed( long seed );
int RandomInt( int max ) ;
float RandomFloat( float max = 1.0 );
unsigned long RandomIntLong( );

#endif _RANDOM_UTILS_HPP_
