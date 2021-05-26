#ifndef _STATS_HPP_
#define _STATS_HPP_

#include "module.hpp"

class Stats : public Module {
  int    _num_samples;
  double _sample_sum;

  int    _num_bins;
  double _bin_size;

  int *_hist;

public:
  Stats( Module *parent, const string &name,
	 double bin_size = 1.0, int num_bins = 10 );
  ~Stats( );

  void Clear( );

  //void SetBinSize( double bin_size );
  //void SetNumBins( int num_bins );

  double Average( ) const;
  int    NumSamples( ) const;

  void AddSample( double val );
  void AddSample( int val );

  void Display( ) const;
};

#endif _STATS_HPP_
