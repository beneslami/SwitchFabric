#ifndef _SELECTFUNC_HPP_
#define _SELECTFUNC_HPP_

#include "router.hpp"
#include "outputset.hpp"
#include "config_utils.hpp"

typedef void (*tSelectionFunction)( const Router *, OutputSet *, 
				    int *, int * );

void InitializeSelectionMap( );
tSelectionFunction GetSelectionFunction( const Configuration& config );

void random_select( const Router *r, OutputSet *outputs,
		    int *out_port, int *out_vc );

#endif _SELECTFUNC_HPP_
