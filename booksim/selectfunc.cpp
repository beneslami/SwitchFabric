#include "booksim.hpp"
#include <map>
#include <stdlib.h>

#include "selectfunc.hpp"
#include "random_utils.hpp"

map<string, tSelectionFunction> gSelectionFunctionMap;

//=============================================================

void random_select( const Router *r, OutputSet *outputs,
		    int *out_port, int *out_vc )
{
  // first select a requested output port

  int num_outputs = outputs->Size( );
  int pri         = RandomInt( num_outputs - 1 );
  int out;

  *out_port = -1;

  for ( int i = 0; i < num_outputs; ++i ) {
    out = ( i + pri ) % num_outputs;

    if ( !outputs->OutputEmpty( out ) ) {
      // now select a random vc from that port
      
      int num_vc   = outputs->NumVCs( out );
      int vc_index = RandomInt( num_vc - 1 );
      
      *out_port = out;
      *out_vc   = outputs->GetVC( out, vc_index );
      break;
    }
  }
}

//=============================================================

void roundrobin_select( const Router *r, OutputSet *outputs,
			int *out_port, int *out_vc )
{
  // first select a requested output port

  // now select a vc for that port
}

//=============================================================

void smallestqueue_select( const Router *r, OutputSet *outputs,
			   int *out_port, int *out_vc )
{
  // first select a requested output port

  // now select a vc for that port
}

//=============================================================

void duato_select( )
{
}

//=============================================================

void InitializeSelectionMap( )
{
  /* Register selection functions here */

  gSelectionFunctionMap["random"] = &random_select;
}

tSelectionFunction GetSelectionFunction( const Configuration& config )
{
  map<string, tSelectionFunction>::const_iterator match;
  tSelectionFunction rf;

  string fn;

  config.GetStr( "selection_function", fn, "random" );
  match = gSelectionFunctionMap.find( fn );

  if ( match != gSelectionFunctionMap.end( ) ) {
    rf = match->second;
  } else {
    cout << "Error: Undefined selection function '" << fn << "'." << endl;
    exit(-1);
  }

  return rf;
}
