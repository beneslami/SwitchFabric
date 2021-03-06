#include "booksim.hpp"
#include <math.h>
#include <iostream>

#include "stats.hpp"

Stats::Stats( Module *parent, const string &name,
	      double bin_size, int num_bins ) :
  Module( parent, name ),
  _num_bins( num_bins ), _bin_size( bin_size )
{
  _hist = new int [_num_bins];

  Clear( );
}

Stats::~Stats( )
{
  delete [] _hist;
}

void Stats::Clear( )
{
  _num_samples = 0;
  _sample_sum  = 0.0;

  for ( int b = 0; b < _num_bins; ++b ) {
    _hist[b] = 0;
  }
}

/*void Stats::SetBinSize( double bin_size )
{
  _bin_size = bin_size;
}

void Stats::SetNumBins( int num_bins )
{
  _num_bins = num_bins;
}*/

double Stats::Average( ) const
{
  return _sample_sum / (double)_num_samples;
}

int Stats::NumSamples( ) const
{
  return _num_samples;
}

void Stats::AddSample( double val )
{
  int b;

  _num_samples++;
  _sample_sum += val;

  b = (int)floor( val / _bin_size );

  if ( b < 0 ) { b = 0; }
  else if ( b >= _num_bins ) { b = _num_bins - 1; }

  _hist[b]++;
}

void Stats::AddSample( int val )
{
  AddSample( (double)val );
}

void Stats::Display( ) const
{
  int b;

  cout << "bins = [ ";
  for ( b = 0; b < _num_bins; ++b ) {
    cout << b*_bin_size << " ";
  }
  cout << "];" << endl;

  cout << "freq = [ ";
  for ( b = 0; b < _num_bins; ++b ) {
    cout << _hist[b] << " ";
  }
  cout << "];" << endl;
}
