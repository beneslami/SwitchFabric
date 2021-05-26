#include "booksim.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <assert.h>

#include "vc_router.hpp"

VCRouter::VCRouter( const Configuration& config,
		    Module *parent, string name, int id,
		    int inputs, int outputs )
  : Router( config,
	    parent, name,
	    id,
	    inputs, outputs )
{
  int i;
  string alloc_type;
  ostringstream vc_name;
  
  _vcs         = config.GetInt( "num_vcs" );
  _vc_size     = config.GetInt( "vc_buf_size" );

  // Routing and selection

  _rf = GetRoutingFunction( config );
  _sf = GetSelectionFunction( config );

  _selected_set = new OutputSet( outputs );

  // Alloc VC's

  _vc = new VC ** [_inputs];

  for ( i = 0; i < _inputs; ++i ) {
    _vc[i] = new VC * [_vcs];

    for ( int v = 0; v < _vcs; ++v ) { // Name the vc modules
      vc_name << "vc_i" << i << "_v" << v;
      _vc[i][v] = new VC( config, _outputs, this, vc_name.str( ) );
      vc_name.seekp( 0, ios::beg );
    }
  }

  // Alloc next VCs' buffer state

  _next_vcs = new BufferState * [_outputs];

  for ( int o = 0; o < _outputs; ++o ) {
    vc_name << "next_vc_o" << o;
    _next_vcs[o] = new BufferState( config, this, vc_name.str( ) );
    vc_name.seekp( 0, ios::beg );
  }

  // Alloc allocators

  config.GetStr( "vc_allocator", alloc_type );
  _vc_allocator = Allocator::NewAllocator( config, 
					   this, "vc_allocator",
					   alloc_type, 
					   _vcs*_inputs, 1,
					   _vcs*_outputs, 1 );

  if ( !_vc_allocator ) {
    cout << "ERROR: Unknown vc_allocator type " << alloc_type << endl;
    exit(-1);
  }

  config.GetStr( "sw_allocator", alloc_type );
  _sw_allocator = Allocator::NewAllocator( config,
					   this, "sw_allocator",
					   alloc_type, 
					   _inputs*_input_speedup, _input_speedup, 
					   _outputs*_output_speedup, _output_speedup );

  if ( !_sw_allocator ) {
    cout << "ERROR: Unknown sw_allocator type " << alloc_type << endl;
    exit(-1);
  }

  _sw_rr_offset = new int [_inputs];
  for ( i = 0; i < _inputs; ++i ) {
    _sw_rr_offset[i] = 0;
  }

  // Alloc pipelines (to simulate processing/transmission delays)

  _crossbar_pipe = 
    new PipelineFIFO<Flit>( this, "crossbar_pipeline", _outputs*_output_speedup, 
			    _st_prepare_delay + _st_final_delay );

  _credit_pipe =
    new PipelineFIFO<Credit>( this, "credit_pipeline", _inputs,
			      _credit_delay );

  // Input and output queues

  _input_buffer  = new queue<Flit *> [_inputs]; 
  _output_buffer = new queue<Flit *> [_outputs]; 

  _in_cred_buffer  = new queue<Credit *> [_inputs]; 
  _out_cred_buffer = new queue<Credit *> [_outputs];

  // Switch configuration (when held for multiple cycles)

  _hold_switch_for_packet = config.GetInt( "hold_switch_for_packet" );
  _switch_hold_in  = new int [_inputs*_input_speedup];
  _switch_hold_out = new int [_outputs*_output_speedup];
  _switch_hold_vc  = new int [_inputs*_input_speedup];

  for ( i = 0; i < _inputs*_input_speedup; ++i ) {
    _switch_hold_in[i] = -1;
    _switch_hold_vc[i] = -1;
  }

  for ( i = 0; i < _outputs*_output_speedup; ++i ) {
    _switch_hold_out[i] = -1;
  }
}

VCRouter::~VCRouter( )
{
  int i, o, v;

  delete _selected_set;

  for ( i = 0; i < _inputs; ++i ) {
    delete [] _vc[i];
  }

  for ( o = 0; o < _outputs; ++o ) {
    delete _next_vcs[o];
  }

  for ( i = 0; i < _inputs; ++i ) {
    for ( v = 0; v < _vcs; ++v ) {
      delete _vc[i][v];
    }
    delete [] _vc[i];
  }

  delete [] _vc;
  delete [] _next_vcs;

  delete _vc_allocator;
  delete _sw_allocator;

  delete [] _sw_rr_offset;

  delete _crossbar_pipe;
  delete _credit_pipe;

  delete [] _input_buffer;
  delete [] _output_buffer;

  delete [] _in_cred_buffer;
  delete [] _out_cred_buffer;

  delete [] _switch_hold_in;
  delete [] _switch_hold_vc;
  delete [] _switch_hold_out;
}
  
void VCRouter::ReadInputs( )
{
  _ReceiveFlits( );
  _ReceiveCredits( );
}

void VCRouter::InternalStep( )
{
  _InputQueuing( );
  _Route( );
  _PreVCAlloc( );
  _InputCredits( );
  _VCAlloc( );
  _SWAlloc( );

  for ( int input = 0; input < _inputs; ++input ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {
      _vc[input][vc]->AdvanceTime( );
    }
  }

  _crossbar_pipe->Advance( );
  _credit_pipe->Advance( );

  _OutputQueuing( );
}

void VCRouter::WriteOutputs( )
{
  _SendFlits( );
  _SendCredits( );
}

void VCRouter::_ReceiveFlits( )
{
  Flit *f;

  for ( int input = 0; input < _inputs; ++input ) { 
    f = *((*_input_channels)[input]);

    if ( f ) {
      _input_buffer[input].push( f );
    }
  }
}

void VCRouter::_ReceiveCredits( )
{
  Credit *c;

  for ( int output = 0; output < _outputs; ++output ) {  
    c = *((*_output_credits)[output]);

    if ( c ) {
      _out_cred_buffer[output].push( c );
    }
  }
}

void VCRouter::_InputQueuing( )
{
  Flit   *f;
  VC     *cur_vc;

  for ( int input = 0; input < _inputs; ++input ) {
    if ( !_input_buffer[input].empty( ) ) {
      f = _input_buffer[input].front( );
      _input_buffer[input].pop( );

      cur_vc = _vc[input][f->vc];

      if ( !cur_vc->AddFlit( f ) ) {
	Error( "VC buffer overflow" );
      }

      if ( f->watch ) {
	cout << "Received flit at " << _fullname << endl;
	cout << *f;
      }
    }
  }

  for ( int input = 0; input < _inputs; ++input ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {
      cur_vc = _vc[input][vc];

      if ( ( cur_vc->GetState( ) == VC::idle ) &&
	   ( !cur_vc->Empty( ) ) ) {
	
	f = cur_vc->FrontFlit( );

	// Advance VC queue state when a head flit arrives
	if ( f->head ) {
	  cur_vc->SetState( VC::routing );
	} else if ( cur_vc->GetState( ) == VC::idle ) {
	  Error( "Received non-head flit at idle VC" );
	}
      }
    }
  }
}

void VCRouter::_Route( )
{
  VC *cur_vc;
  Flit *f;

  for ( int input = 0; input < _inputs; ++input ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {

      cur_vc = _vc[input][vc];

      if ( ( cur_vc->GetState( ) == VC::routing ) &&
	   ( cur_vc->GetStateTime( ) >= _routing_delay ) ) {

	// ROUTE
	f = cur_vc->FrontFlit( );
	if ( !f->head ) {
	  Error( "First flit not head for routing!" );
	}

	cur_vc->Route( _rf, this, f, input );
	cur_vc->SetState( VC::vc_alloc );
      }
    }
  }
}

void VCRouter::_AddVCRequests( VC* cur_vc, int input_index, 
			       bool watch, bool remove )
{
  const OutputSet *route_set;
  int vc_cnt, out_vc;
  int in_priority, out_priority;
  bool empty;

  route_set    = cur_vc->GetRouteSet( );
  out_priority = cur_vc->GetPriority( );

  empty = true;
  for ( int output = 0; output < _outputs; ++output ) {
    vc_cnt = route_set->NumVCs( output );

    if ( vc_cnt > 0 ) {
      empty = false;
    }

    for ( int vc_index = 0; vc_index < vc_cnt; ++vc_index ) {
      out_vc = route_set->GetVC( output, vc_index, &in_priority );
     
      // On the input input side, a VC might request several output 
      // VCs.  These VCs can be prioritized by the routing function
      // are this is reflected in "in_priority".  On the output,
      // if multiple VCs are requesting the same output VC, the priority
      // of VCs is based on the actual packet priorities, which is
      // reflected in "out_priority".

      //if ( dest_vc->IsAvailableFor( out_vc ) ) {

      if ( remove ) {
	_vc_allocator->RemoveRequest( input_index, output*_vcs + out_vc, 1 );
      } else {
	_vc_allocator->AddRequest( input_index, output*_vcs + out_vc, 1, 
				   in_priority, out_priority );
      }

	/*if ( watch ) {
	  cout << "available" << endl;
	}
      } else if ( watch ) {
	cout << "busy" << endl;
	}*/
    }
  }


  if ( empty ) {
    Flit *f = cur_vc->FrontFlit( );
    cout << *f;
    Error( "Empty VC request set" );
  }
}

void VCRouter::_PreVCAlloc( )
{
  VC   *cur_vc;
  int  input_index;
  Flit *f;

  for ( int input = 0; input < _inputs; ++input ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {
      
      cur_vc = _vc[input][vc];
      
      if ( ( cur_vc->GetState( ) == VC::vc_alloc ) &&
	   ( cur_vc->GetStateTime( ) == _vc_alloc_delay ) ) {
	// Minimum delay has passed, request VC

	f = cur_vc->FrontFlit( );
	input_index = input*_vcs + vc;

	if ( f->watch ) {
	  cout << "VC requesting allocation at " << _fullname << endl;
	  cout << "  input_index = " << input_index << endl;
	  cout << *f;
	}

	_AddVCRequests( cur_vc, input_index, f->watch );
      }
    }
  }
}

void VCRouter::_InputCredits( )
{
  BufferState *dest_vc;
  Credit *c;
  int vc;

  for ( int output = 0; output < _outputs; ++output ) {
    if ( !_out_cred_buffer[output].empty( ) ) {
      c = _out_cred_buffer[output].front( );
      _out_cred_buffer[output].pop( );
      
      dest_vc = _next_vcs[output];
      dest_vc->ProcessCredit( c );
      
      for ( int v = 0; v < c->vc_cnt; ++v ) {
	vc = c->vc[v];

	if ( dest_vc->IsAvailableFor( vc ) ) {
	  _vc_allocator->MaskOutput( output*_vcs + vc, 0 );
	}
      }

      delete c;
    }
  }
}

void VCRouter::_VCAlloc( )
{
  VC          *cur_vc;
  BufferState *dest_vc;
  int         input_and_vc;
  int         match_input;
  int         match_vc;

  Flit        *f;

  /*if ( _id == 48 ) {
    cout << "requests at router " << _fullname << endl;
    _vc_allocator->PrintRequests( );
    }*/

  _vc_allocator->Allocate( );

  // Winning flits get a VC

  for ( int output = 0; output < _outputs; ++output ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {
      input_and_vc = _vc_allocator->InputAssigned( output*_vcs + vc );

      if ( input_and_vc != -1 ) {
	match_input = input_and_vc / _vcs;
	match_vc    = input_and_vc - match_input*_vcs;

	cur_vc  = _vc[match_input][match_vc];
	dest_vc = _next_vcs[output];

	cur_vc->SetState( VC::active );
	cur_vc->SetOutput( output, vc );
	dest_vc->TakeBuffer( vc );

	_vc_allocator->MaskOutput( output*_vcs + vc, 1 );

	f = cur_vc->FrontFlit( );
	
	if ( f->watch ) {
	  cout << "Granted VC allocation at " << _fullname 
	       << " (input index " << input_and_vc << "," 
	       << " output index " << output*_vcs + vc << ")" << endl;
	  cout << *f;
	}

	_AddVCRequests( cur_vc, input_and_vc, false, true );
      }
    }
  }
}

void VCRouter::_SWAlloc( )
{
  Flit        *f;
  Credit      *c;

  VC          *cur_vc;
  BufferState *dest_vc;

  int input;
  int output;
  int vc;

  int expanded_input;
  int expanded_output;

  _sw_allocator->Clear( );

  for ( input = 0; input < _inputs; ++input ) {
    // Arbitrate (round-robin) between multiple 
    // requesting VCs at the same input (handles 
    // the case when multiple VC's are requesting
    // the same output port)
    vc = _sw_rr_offset[ input ];

    for ( int v = 0; v < _vcs; ++v ) {
      cur_vc = _vc[input][vc];

      if ( ( cur_vc->GetState( ) == VC::active ) && 
	   ( !cur_vc->Empty( ) ) ) {

	dest_vc = _next_vcs[cur_vc->GetOutputPort( )];

	if ( !dest_vc->IsFullFor( cur_vc->GetOutputVC( ) ) ) {
	  
	  // When input_speedup > 1, the virtual channel buffers
	  // are interleaved to create multiple input ports to
	  // the switch.  Similarily, the output ports are
	  // interleaved based on their originating input when
	  // output_speedup > 1.

	  expanded_input  = (vc%_input_speedup)*_inputs + input;
	  expanded_output = (input%_output_speedup)*_outputs + cur_vc->GetOutputPort( );

	  if ( ( _switch_hold_in[expanded_input] == -1 ) && 
	       ( _switch_hold_out[expanded_output] == -1 ) ) {

	    // We could have requested this same input-output pair in a previous
	    // iteration, only replace the previous request if the current
	    // request has a higher priority (this is default behavior of the
	    // allocators).  Switch allocation priorities are strictly 
	    // determined by the packet priorities.

	    _sw_allocator->AddRequest( expanded_input, expanded_output, vc, 
				       cur_vc->GetPriority( ), 
				       cur_vc->GetPriority( ) );
	  }
	}
      }
      vc = ( vc + 1 ) % _vcs;
    }
  }

  _sw_allocator->Allocate( );

  // Winning flits cross the switch

  _crossbar_pipe->WriteAll( 0 );

  for ( input = 0; input < _inputs; ++input ) {
    c = 0;

    for ( int s = 0; s < _input_speedup; ++s ) {

      expanded_input  = s*_inputs + input;

      if ( _switch_hold_in[expanded_input] != -1 ) {
	expanded_output = _switch_hold_in[expanded_input];
	vc = _switch_hold_vc[expanded_input];
	cur_vc = _vc[input][vc];
	
	if ( cur_vc->Empty( ) ) { // Cancel held match if VC is empty
	  expanded_output = -1;
	}
      } else {
	expanded_output = _sw_allocator->OutputAssigned( expanded_input );
      }

      if ( expanded_output >= 0 ) {
	output = expanded_output % _outputs;

	if ( _switch_hold_in[expanded_input] == -1 ) {
	  vc = _sw_allocator->ReadRequest( expanded_input, expanded_output );
	  cur_vc = _vc[input][vc];
	}

	if ( _hold_switch_for_packet ) {
	  _switch_hold_in[expanded_input] = expanded_output;
	  _switch_hold_vc[expanded_input] = vc;
	  _switch_hold_out[expanded_output] = expanded_input;
	}

	assert( ( cur_vc->GetState( ) == VC::active ) && 
		( !cur_vc->Empty( ) ) && 
		( cur_vc->GetOutputPort( ) == ( expanded_output % _outputs ) ) );
	
	dest_vc = _next_vcs[cur_vc->GetOutputPort( )];
	
	assert( !dest_vc->IsFullFor( cur_vc->GetOutputVC( ) ) );
	
	// Forward flit to crossbar and send credit back
	f = cur_vc->RemoveFlit( );

	f->hops++;

	if ( f->watch ) {
	  cout << "Forwarding flit through crossbar at " << _fullname << ":" << endl;
	  cout << *f;
	}
	
	if ( !c ) {
	  c = _NewCredit( _vcs );
	}

	c->vc[c->vc_cnt] = f->vc;
	c->vc_cnt++;
	
	f->vc = cur_vc->GetOutputVC( );
	dest_vc->SendingFlit( f );
	
	_crossbar_pipe->Write( f, expanded_output );
	
	if ( f->tail ) {
	  cur_vc->SetState( VC::idle );

	  _switch_hold_in[expanded_input] = -1;
	  _switch_hold_vc[expanded_input] = -1;
	  _switch_hold_out[expanded_output] = -1;
	}

	_sw_rr_offset[input] = ( f->vc + 1 ) % _vcs;
      } 
    }
    
    _credit_pipe->Write( c, input );
  }
}

void VCRouter::_OutputQueuing( )
{
  Flit   *f;
  Credit *c;
  int expanded_output;

  for ( int output = 0; output < _outputs; ++output ) {
    for ( int t = 0; t < _output_speedup; ++t ) {
      expanded_output = _outputs*t + output;
      f = _crossbar_pipe->Read( expanded_output );

      if ( f ) {
	_output_buffer[output].push( f );
      }
    }
  }  

  for ( int input = 0; input < _inputs; ++input ) {
    c = _credit_pipe->Read( input );

    if ( c ) {
      _in_cred_buffer[input].push( c );
    }
  }
}

void VCRouter::_SendFlits( )
{
  Flit *f;

  for ( int output = 0; output < _outputs; ++output ) {
    if ( !_output_buffer[output].empty( ) ) {
      f = _output_buffer[output].front( );
      _output_buffer[output].pop( );
    } else {
      f = 0;
    }
	
    *(*_output_channels)[output] = f;
  }
}

void VCRouter::_SendCredits( )
{
  Credit *c;

  for ( int input = 0; input < _inputs; ++input ) {
    if ( !_in_cred_buffer[input].empty( ) ) {
      c = _in_cred_buffer[input].front( );
      _in_cred_buffer[input].pop( );
    } else {
      c = 0;
    }

    *(*_input_credits)[input] = c;
  }
}

void VCRouter::Display( ) const
{
  for ( int input = 0; input < _inputs; ++input ) {
    for ( int v = 0; v < _vcs; ++v ) {
      _vc[input][v]->Display( );
    }
  }
}
