#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "saf_router.hpp"

SAFRouter::SAFRouter( const Configuration& config,

		    Module *parent, string name,

		    int id,

		    int inputs, int outputs,

		    const vector<Flit **>   *input_channels,
		    const vector<Credit **> *credits_in,

		    vector<Flit **>   *output_channels,
		    vector<Credit **> *credits_out )
  : Router( config,
	    parent, name,
	    id,
	    inputs, outputs,
	    input_channels, credits_in,
	    output_channels, credits_out )
{
  string alloc_type;
  ostringstream buf_name;
  
  _rf      = GetRoutingFunction( config );

  // Alloc next buffer states

  _next_buf = new BufferState [_outputs] ( config );

  for ( int o = 0; o < _outputs; ++o ) {
    buf_name << "next_buffer_o" << o;
    _next_buf[o].SetName( this, buf_name.str( ) );
    buf_name.seekp( 0, ios::beg );
  }

  // Alloc allocators

  config.GetStr( "sw_allocator", alloc_type );
  _sw_allocator = Allocator::NewAllocator( this, "sw_allocator",
					   alloc_type, 
					   _inputs, 
					   _outputs );

  if ( !_sw_allocator ) {
    cout << "ERROR: Unknown sw_allocator type " << alloc_type << endl;
    exit(-1);
  }

  // Alloc pipelines (to simulate processing/transmission delays)

  _crossbar_pipe = 
    new PipelineFIFO<Flit>( this, "crossbar_pipeline", _outputs, 
			    _st_prepare_delay + _st_final_delay );

  _credit_pipe =
    new PipelineFIFO<Credit>( this, "credit_pipeline", _inputs,
			      _credit_delay );

  queue<Flit *> _buffer;
}

SAFRouter::~SAFRouter( )
{
  delete [] _next_buf;

  delete _sw_allocator;

  delete _crossbar_pipe;
  delete _credit_pipe;
}
  
void SAFRouter::ReadInputs( )
{
  _ReceiveFlits( );
  _ReceiveCredits( );
  _Route( );
  _SWAlloc( );

  _crossbar_pipe->Advance( );
  _credit_pipe->Advance( );

  _OutputQueuing( );
}

void SAFRouter::WriteOutputs( )
{
  _SendFlits( );
  _SendCredits( );
}

void SAFRouter::_ReceiveFlits( )
{
  Flit *f;
  VC   *cur_vc;

  for ( int input = 0; input < _inputs; ++input ) {
    
    f = *((*_input_channels)[input]);

    if ( f ) {
      cout << "received flit at " << _fullname << ", id = " 
	   << f->id << endl;

      // push flit into queue

    }
  }
}

void SAFRouter::_ReceiveCredits( )
{
  Credit *c;

  for ( int output = 0; output < _outputs; ++output ) {
    
    c = *((*_credits_out)[output]);

    if ( c ) {
      _next_bufs[output].ProcessCredit( c );
    }
  }
}

void SAFRouter::_Route( )
{
  VC *cur_vc;

  for ( int input = 0; input < _inputs; ++input ) {
    
    // pop out of routing pipeline 
    // and into the appropriate buffer class

  }
}

void SAFRouter::_SWAlloc( )
{
  Flit        *f;
  Credit      *c;

  VC          *cur_vc;
  BufferState *dest_vc;

  int  input;
  int  vc;

  _sw_allocator->Clear( );

  for ( input = 0; input < _inputs; ++input ) {
    
    _sw_allocator->AddRequest( input, cur_vc->DestPort( ) );
  }

  //_sw_allocator->PrintRequests( );
  _sw_allocator->Allocate( );

  // Winning flits cross the switch

  _credit_pipe->WriteAll( 0 );

  for ( int output = 0; output < _outputs; ++output ) {
    
    input = _sw_allocator->InputAssigned( output );
    if ( input >= 0 ) {

      // FIX this random offset thing

      int rand_offset = lrand48( );

      for ( vc = 0; vc < _vcs; ++vc ) {
	cur_vc = &_vc[input][(vc+rand_offset)%_vcs];
	
	if ( ( cur_vc->GetState( ) == VC::active ) && 
	   ( !cur_vc->Empty( ) ) && ( cur_vc->DestPort( ) == output ) ) {

	  dest_vc = &_next_vcs[cur_vc->DestPort( )];

	  if ( !dest_vc->IsFullFor( cur_vc->DestVC( ) ) ) {
	    break;
	  }
	}
      }
      
      if ( vc == _vcs ) {
	Error( "Mismatch between switch request and allocation" );
      }

      // forward flit to crossbar and send credit back
      f = cur_vc->RemoveFlit( );

      c     = _NewCredit( );
      c->vc = f->vc;

      f->vc = cur_vc->DestVC( );
      dest_vc->SendingFlit( f );

      _crossbar_pipe->Write( f, output );
      _credit_pipe->Write( c, input );

      cout << "pushing credit for input " << input << endl;

      if ( f->tail ) {
	cur_vc->SetState( VC::idle );
      }

    } else {
      _crossbar_pipe->Write( 0, output );
    }
  }
}

void SAFRouter::_OutputQueuing( )
{
  
}

void SAFRouter::_SendFlits( )
{
  Flit *f;

  for ( int output = 0; output < _outputs; ++output ) {
    f = _crossbar_pipe->Read( output );
    *(*_output_channels)[output] = f;
  }
}

void SAFRouter::_SendCredits( )
{
  Credit *c;

  for ( int input = 0; input < _inputs; ++input ) {
    c = _credit_pipe->Read( input );
    *(*_credits_in)[input] = c;
  }
}

void SAFRouter::Display( ) const
{
  for ( int input = 0; input < _inputs; ++input ) {
    for ( int v = 0; v < _vcs; ++v ) {
      _vc[input][v].Display( );
    }
  }
}
