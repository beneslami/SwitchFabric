#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "oq_router.hpp"

OQRouter::OQRouter( const Configuration& config,
		    Module *parent, string name, int id,
		    int inputs, int outputs )
  : Router( config,
	    parent, name,
	    id,
	    inputs, outputs )
{
  string alloc_type;
  ostringstream vc_name;
  
  _vcs     = config.GetInt( "num_vcs" );
  _vc_size = config.GetInt( "vc_buf_size" );

  // Routing and selection

  _rf = GetRoutingFunction( config );
  _sf = GetSelectionFunction( config );

  _selected_set = new OutputSet( outputs );

  // Alloc VC's

  _vc = new VC * [_inputs];

  for ( int i = 0; i < _inputs; ++i ) {
    _vc[i] = new VC [_vcs] ( config, _outputs );

    for ( int v = 0; v < _vcs; ++v ) { // Name the vc modules
      vc_name << "vc_i" << i << "_v" << v;
      _vc[i][v].SetName( this, vc_name.str( ) );
      vc_name.seekp( 0, ios::beg );
    }
  }

  // Alloc next VCs' buffer state

  _next_vcs = new BufferState [_outputs] ( config );

  for ( int o = 0; o < _outputs; ++o ) {
    vc_name << "next_vc_o" << o;
    _next_vcs[o].SetName( this, vc_name.str( ) );
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
  for ( int i = 0; i < _inputs; ++i ) {
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

  for ( int i = 0; i < _inputs*_input_speedup; ++i ) {
    _switch_hold_in[i] = -1;
    _switch_hold_vc[i] = -1;
  }

  for ( int i = 0; i < _outputs*_output_speedup; ++i ) {
    _switch_hold_out[i] = -1;
  }
}

OQRouter::~OQRouter( )
{
  delete _selected_set;

  for ( int i = 0; i < _inputs; ++i ) {
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
  
void OQRouter::ReadInputs( )
{
  _ReceiveFlits( );
  _ReceiveCredits( );
}

void OQRouter::InternalStep( )
{
  _InputQueuing( );
  _Route( );
  //_VCAlloc( );
  //_SWAlloc( );

  //_crossbar_pipe->Advance( );
  _credit_pipe->Advance( );

  _OutputQueuing( );
}

void OQRouter::WriteOutputs( )
{
  _SendFlits( );
  _SendCredits( );
}

void OQRouter::_ReceiveFlits( )
{
  Flit *f;

  for ( int input = 0; input < _inputs; ++input ) { 
    f = *((*_input_channels)[input]);

    if ( f ) {
      _input_buffer[input].push( f );
    }
  }
}

void OQRouter::_ReceiveCredits( )
{
  Credit *c;

  for ( int output = 0; output < _outputs; ++output ) {  
    c = *((*_output_credits)[output]);

    if ( c ) {
      _out_cred_buffer[output].push( c );
    }
  }
}

void OQRouter::_InputQueuing( )
{
  Flit   *f;
  Credit *c;
  VC     *cur_vc;

  for ( int input = 0; input < _inputs; ++input ) {
    if ( !_input_buffer[input].empty( ) ) {
      f = _input_buffer[input].front( );
      _input_buffer[input].pop( );

      if ( f->watch ) {
	cout << "Received flit at " << _fullname << endl;
	cout << *f;
      }

      cur_vc = &_vc[input][f->vc];

      if ( !cur_vc->AddFlit( f ) ) {
	Error( "VC buffer overflow" );
      }
    }
  }
      
  for ( int input = 0; input < _inputs; ++input ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {

      cur_vc = &_vc[input][vc];
      
      if ( cur_vc->GetState( ) == VC::idle ) {
	f = cur_vc->FrontFlit( );

	if ( f ) {
	  if ( !f->head ) {
	    Error( "Received non-head flit at idle VC" );
	  }

	  cur_vc->Route( _rf, this, f );

	  //_rf( (Router *)this, f, _route_set, false );
	  //_sf( (Router *)this, _route_set, &dest_port, &dest_vc );
	  
	  //cur_vc->SetDestPort( dest_port );
	  //cur_vc->SetDestVC( dest_vc );
	  cur_vc->SetState( VC::routing );
	}
      }
    }
  }  

  for ( int output = 0; output < _outputs; ++output ) {
    if ( !_out_cred_buffer[output].empty( ) ) {
      c = _out_cred_buffer[output].front( );
      _out_cred_buffer[output].pop( );
   
      _next_vcs[output].ProcessCredit( c );
    }
  }
}

void OQRouter::_Route( )
{
  VC *cur_vc;

  for ( int input = 0; input < _inputs; ++input ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {

      cur_vc = &_vc[input][vc];

      if ( ( cur_vc->GetState( ) == VC::routing ) &&
	   ( cur_vc->GetStateTime( ) >= _routing_delay ) ) {

	cur_vc->SetState( VC::vc_alloc );
      }
    }
  }
}

void OQRouter::_AddVCRequests( VC* cur_vc, int input_index )
{
  const OutputSet *route_set;
  BufferState *dest_vc;
  int vc_cnt, out_vc;

  route_set = cur_vc->GetRouteSet( );

  //cout << "cur_vc = " << cur_vc << endl;
  for ( int output = 0; output < _outputs; ++output ) {
    vc_cnt = route_set->NumVCs( output );
    dest_vc = &_next_vcs[output];

    //cout << "  output " << output << ", vc_cnt = " << vc_cnt << endl;

    for ( int vc_index = 0; vc_index < vc_cnt; ++vc_index ) {
      out_vc = route_set->GetVC( output, vc_index );
      if ( dest_vc->IsAvailableFor( out_vc ) ) {
	_vc_allocator->AddRequest( input_index, output*_vcs + out_vc );
      }
    }
  }
}

void OQRouter::_VCAlloc( )
{
  VC          *cur_vc;
  BufferState *dest_vc;
  int         input_and_vc;
  int         match_input;
  int         match_vc;

  _vc_allocator->Clear( );

  for ( int input = 0; input < _inputs; ++input ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {

      cur_vc = &_vc[input][vc];

      if ( ( cur_vc->GetState( ) == VC::vc_alloc ) &&
	   ( cur_vc->GetStateTime( ) >= _vc_alloc_delay ) ) {

	_AddVCRequests( cur_vc, input*_vcs + vc );
      }
    }
  }

  //_vc_allocator->PrintRequests( );
  _vc_allocator->Allocate( );

  // Winning flits get a VC

  for ( int output = 0; output < _outputs; ++output ) {
    for ( int vc = 0; vc < _vcs; ++vc ) {
      input_and_vc = _vc_allocator->InputAssigned( output*_vcs + vc );

      if ( input_and_vc != -1 ) {
	match_input = input_and_vc / _vcs;
	match_vc    = input_and_vc - match_input*_vcs;

	cur_vc  = &_vc[match_input][match_vc];
	dest_vc = &_next_vcs[output];

	cur_vc->SetState( VC::active );
	cur_vc->SetOutput( output, vc );
	dest_vc->TakeBuffer( vc );
      }
    }
  }
}

void OQRouter::_SWAlloc( )
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

    // Arbitrate between multiple requesting VCs at the same input
    //int rand_offset = lrand48( );

    // THIS DOES NOT WORK WITH INPUT SPEEDUP YET!!!!
    //vc = lrand48( ) % _vcs; 
    vc = _sw_rr_offset[ input ];

    for ( int v = 0; v < _vcs; ++v ) {
      //vc = (v+rand_offset)%_vcs;
      cur_vc = &_vc[input][vc];

      if ( ( cur_vc->GetState( ) == VC::active ) && 
	   ( !cur_vc->Empty( ) ) ) {

	dest_vc = &_next_vcs[cur_vc->GetOutputPort( )];

	if ( !dest_vc->IsFullFor( cur_vc->GetOutputVC( ) ) ) {
	  
	  //
	  // When input_speedup > 1, the virtual channel buffers
	  // are interleaved to create multiple input ports to
	  // the switch.  Similarily, the output ports are
	  // interleaved based on their originating input when
	  // output_speedup > 1.
	  //

	  expanded_input  = (vc%_input_speedup)*_inputs + input;
	  expanded_output = (input%_output_speedup)*_outputs + cur_vc->GetOutputPort( );

	  if ( ( _switch_hold_in[expanded_input] == -1 ) && 
	       ( _switch_hold_out[expanded_output] == -1 ) && 
	       ( _sw_allocator->ReadRequest( expanded_input, expanded_output ) == -1 ) ) {
	    _sw_allocator->AddRequest( expanded_input, expanded_output, vc );
	  }
	}
      }
      vc = ( vc + 1 ) % _vcs;
    }
  }

  /*cout << endl << "held config: ";
  for ( int i = 0; i < _inputs; ++i ) {
    cout << _switch_hold_in[i] << " ";
  }
  cout << endl;*/

  /*if ( _id == 0 ) {
    _sw_allocator->PrintRequests( );
    }*/
  _sw_allocator->Allocate( );

  // Winning flits cross the switch

  _crossbar_pipe->WriteAll( 0 );

  for ( int input = 0; input < _inputs; ++input ) {
    c = 0;

    for ( int s = 0; s < _input_speedup; ++s ) {

      expanded_input  = s*_inputs + input;

      if ( _switch_hold_in[expanded_input] != -1 ) {
	expanded_output = _switch_hold_in[expanded_input];
	vc = _switch_hold_vc[expanded_input];
	cur_vc = &_vc[input][vc];
	
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
	  cur_vc = &_vc[input][vc];
	}

	if ( _hold_switch_for_packet ) {
	  _switch_hold_in[expanded_input] = expanded_output;
	  _switch_hold_vc[expanded_input] = vc;
	  _switch_hold_out[expanded_output] = expanded_input;
	}

	assert( ( cur_vc->GetState( ) == VC::active ) && 
		( !cur_vc->Empty( ) ) && 
		( cur_vc->GetOutputPort( ) == ( expanded_output % _outputs ) ) );
	
	dest_vc = &_next_vcs[cur_vc->GetOutputPort( )];
	
	assert( !dest_vc->IsFullFor( cur_vc->GetOutputVC( ) ) );
	
	// Forward flit to crossbar and send credit back
	f = cur_vc->RemoveFlit( );

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
    
    //if (c) { cout << "pushing credit for input " << input << endl; }
    _credit_pipe->Write( c, input );
  }
}

void OQRouter::_OutputQueuing( )
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

void OQRouter::_SendFlits( )
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

void OQRouter::_SendCredits( )
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

void OQRouter::Display( ) const
{
  for ( int input = 0; input < _inputs; ++input ) {
    for ( int v = 0; v < _vcs; ++v ) {
      _vc[input][v].Display( );
    }
  }
}
