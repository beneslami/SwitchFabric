#include "booksim.hpp"
#include <sstream>
#include <math.h>

#include "trafficmanager.hpp"
#include "random_utils.hpp" 

TrafficManager::TrafficManager( const Configuration &config, Network *net )
  : Module( 0, "traffic_manager" )
{
  int s;
  ostringstream tmp_name;
  string sim_type, priority;

  _net    = net;
  _cur_id = 0;

  _sources = _net->NumSources( );
  _dests   = _net->NumDests( );

  // ============ Message priorities ============ 

  config.GetStr( "priority", priority );

  _classes = 1;

  if ( priority == "class" ) {
    _classes  = 2;
    _pri_type = class_based;
  } else if ( priority == "age" ) {
    _pri_type = age_based;
  } else if ( priority == "none" ) {
    _pri_type = none;
  } else {
    Error( "Unknown priority " + priority );
  }

  // ============ Injection VC states  ============ 

  _buf_states = new BufferState * [_sources];

  for ( s = 0; s < _sources; ++s ) {
    tmp_name << "buf_state_" << s;
    _buf_states[s] = new BufferState( config, this, tmp_name.str( ) );
    tmp_name.seekp( 0, ios::beg );
  }

  // ============ Injection queues ============ 

  _time               = 0;
  _warmup_time        = -1;
  _drain_time         = -1;
  _empty_network      = false;

  _measured_in_flight = 0;
  _total_in_flight    = 0;

  _qtime              = new int * [_sources];
  _qdrained           = new bool * [_sources];
  _partial_packets    = new list<Flit *> * [_sources];

  for ( s = 0; s < _sources; ++s ) {
    _qtime[s]           = new int [_classes];
    _qdrained[s]        = new bool [_classes];
    _partial_packets[s] = new list<Flit *> [_classes];
  }

  _voqing = config.GetInt( "voq" );

  // ============ Statistics ============ 

  _latency_stats   = new Stats * [_classes];
  _overall_latency = new Stats * [_classes];

  for ( int c = 0; c < _classes; ++c ) {
    tmp_name << "latency_stat_" << c;
    _latency_stats[c] = new Stats( this, tmp_name.str( ), 1.0, 1000 );
    tmp_name.seekp( 0, ios::beg );

    tmp_name << "overall_latency_stat_" << c;
    _overall_latency[c] = new Stats( this, tmp_name.str( ), 1.0, 1000 );
    tmp_name.seekp( 0, ios::beg );  
  }

  _pair_latency = new Stats * [_dests];
  _hop_stats    = new Stats( this, "hop_stats", 1.0, 20 );;

  _accepted_packets = new Stats * [_dests];

  _overall_accepted     = new Stats( this, "overall_acceptance" );
  _overall_accepted_min = new Stats( this, "overall_min_acceptance" );

  for ( int i = 0; i < _dests; ++i ) {
    tmp_name << "pair_stat_" << i;
    _pair_latency[i] = new Stats( this, tmp_name.str( ), 1.0, 250 );
    tmp_name.seekp( 0, ios::beg );

    tmp_name << "accepted_stat_" << i;
    _accepted_packets[i] = new Stats( this, tmp_name.str( ) );
    tmp_name.seekp( 0, ios::beg );    
  }

  // ============ Simulation parameters ============ 

  _load = config.GetFloat( "injection_rate" ); 
  _packet_size = config.GetInt( "const_flits_per_packet" );

  _total_sims = config.GetInt( "sim_count" );

  _internal_speedup = config.GetFloat( "internal_speedup" );
  _partial_internal_cycles = 0.0;

  _traffic_function  = GetTrafficFunction( config );
  _routing_function  = GetRoutingFunction( config );
  _injection_process = GetInjectionProcess( config );

  config.GetStr( "sim_type", sim_type );

  if ( sim_type == "latency" ) {
    _sim_mode = latency;
  } else if ( sim_type == "throughput" ) {
    _sim_mode = throughput;
  } else {
    Error( "Unknown sim_type " + sim_type );
  }

  _sample_period = config.GetInt( "sample_period" );

  _max_samples    = config.GetInt( "max_samples" );
  _warmup_periods = config.GetInt( "warmup_periods" );

  _latency_thres = config.GetFloat( "latency_thres" );

  _include_queuing = config.GetInt( "include_queuing" );
}

TrafficManager::~TrafficManager( )
{
  for ( int s = 0; s < _sources; ++s ) {
    delete [] _qtime[s];
    delete [] _qdrained[s];
    delete [] _partial_packets[s];
    delete _buf_states[s];
  }

  delete [] _buf_states;
  delete [] _qtime;
  delete [] _qdrained;
  delete [] _partial_packets;

  for ( int c = 0; c < _classes; ++c ) {
    delete _latency_stats[c];
    delete _overall_latency[c];
  }

  delete [] _latency_stats;
  delete [] _overall_latency;

  delete _hop_stats;
  delete _overall_accepted;
  delete _overall_accepted_min;

  for ( int i = 0; i < _dests; ++i ) {
    delete _accepted_packets[i];
    delete _pair_latency[i];
  }

  delete [] _accepted_packets;
  delete [] _pair_latency;
}

Flit *TrafficManager::_NewFlit( )
{
  Flit *f;
  f = new Flit;

  f->id    = _cur_id;
  f->hops  = 0;

  if ( 0 ) {
    f->watch = true;
  } else {
    f->watch = false;
  }

  _in_flight[_cur_id] = true;
  ++_cur_id;

  return f;
}

void TrafficManager::_RetireFlit( Flit *f, int dest )
{
  static int sample_num = 0;

  //hash_map<int, bool>::iterator match;
  map<int, bool>::iterator match;

  match = _in_flight.find( f->id );

  if ( match != _in_flight.end( ) ) {
    if ( f->watch ) {
      cout << "Matched flit ID = " << f->id << endl;
    }
    _in_flight.erase( match );
  } else {
    cout << "Unmatched flit! ID = " << f->id << endl;
    Error( "" );
  }
  
  if ( f->watch ) { //|| (  _time - f->time  > 30 ) ) {
    cout << "Ejecting flit " << f->id 
	 << ",  lat = " << _time - f->time 
	 << ", src = " << f->src 
	 << ", dest = " << f->dest << endl;
  }

  if ( f->head && ( f->dest != dest ) ) {
    cout << "At output " << dest << endl;
    cout << *f;
    Error( "Flit arrived at incorrect output" );
  }

  if ( f->tail ) {
    _total_in_flight--;
    if ( _total_in_flight < 0 ) {
      Error( "Total in flight count dropped below zero!" );
    }

    // Only record statistics once per packet (at tail)
    // and based on the simulation state1
    if ( ( _sim_state == warming_up ) || f->record ) {
      
      _hop_stats->AddSample( f->hops );

      switch( _pri_type ) {
      case class_based:
	_latency_stats[f->pri]->AddSample( _time - f->time );
	break;
      case age_based: // fall through
      case none:
	_latency_stats[0]->AddSample( _time - f->time );
	break;
      }
      
      ++sample_num;
      //cout << "lat(" << sample_num << ") = " << _time - f->time << ";" << endl;
      
      if ( f->src == 0 ) {
	_pair_latency[dest]->AddSample( _time - f->time );
      }
      
      if ( f->record ) {
	_measured_in_flight--;
	if ( _measured_in_flight < 0 ){ 
	  Error( "Measured in flight count dropped below zero!" );
	}
      }
    }
  }

  delete f;
}

int TrafficManager::_IssuePacket( int source, int cl ) const
{
  float class_load;

  if ( _pri_type == class_based ) {
    if ( cl == 0 ) {
      class_load = 0.9 * _load;
    } else {
      class_load = 0.1 * _load;
    }
  } else {
    class_load = _load;
  }
 
  return _injection_process( source, class_load );
}

void TrafficManager::_GeneratePacket( int source, int size, 
				      int cl, int time )
{
  Flit *f;
  bool record;

  if ( ( _sim_state == running ) ||
       ( ( _sim_state == draining ) && ( time < _drain_time ) ) ) {
    ++_measured_in_flight;
    record = true;
  } else {
    record = false;
  }
  ++_total_in_flight;

  for ( int i = 0; i < size; ++i ) {
    f = _NewFlit( );
      
    f->src    = source;
    f->time   = time;
    f->record = record;

    if ( i == 0 ) { // Head flit
      f->head = true;
      f->dest = _traffic_function( source, _net->NumDests( ) );
    } else {
      f->head = false;
      f->dest = -1;
    }

    switch( _pri_type ) {
    case class_based:
      f->pri = cl; break;
    case age_based:
      f->pri = -time; break;
    case none:
      f->pri = 0; break;
    }

    if ( i == ( size - 1 ) ) { // Tail flit
      f->tail = true;
    } else {
      f->tail = false;
    }
    
    f->vc  = -1;

    if ( f->watch ) { 
      cout << "Generating flit at time " << time << endl;
      cout << *f;
    }

    _partial_packets[source][cl].push_back( f );
  }
}

void TrafficManager::_FirstStep( )
{  
  // Ensure that all outputs are defined before starting simulation
  
  _net->WriteOutputs( );
  
  for ( int output = 0; output < _net->NumDests( ); ++output ) {
    _net->WriteCredit( 0, output );
  }
}

void TrafficManager::_Step( )
{
  Flit   *f, *nf;
  Credit *cred;
  int    psize;

  // Receive credits and inject new traffic
  for ( int input = 0; input < _net->NumSources( ); ++input ) {
    cred = _net->ReadCredit( input );
    if ( cred ) {
      _buf_states[input]->ProcessCredit( cred );
      delete cred;
    }
    
    bool write_flit    = false;
    int  highest_class = 0;
    bool generated;

    for ( int c = 0; c < _classes; ++c ) {
      // Potentially generate packets for any (input,class)
      // that is currently empty
      if ( _partial_packets[input][c].empty( ) ) {
	generated = false;
	  
	if ( !_empty_network ) {
	  while( !generated && ( _qtime[input][c] <= _time ) ) {
	    psize = _IssuePacket( input, c );

	    if ( psize ) {
	      _GeneratePacket( input, psize, c, 
			       _include_queuing ? 
			       _qtime[input][c] : _time );
	      generated = true;
	    }
	    ++_qtime[input][c];
	  }
	  
	  if ( ( _sim_state == draining ) && 
	       ( _qtime[input][c] > _drain_time ) ) {
	    _qdrained[input][c] = true;
	  }
	}
	
	if ( generated ) {
	  highest_class = c;
	}
      } else {
	highest_class = c;
      }
    }

    // Now, check partially issued packet to
    // see if it can be issued
    if ( !_partial_packets[input][highest_class].empty( ) ) {
      f = _partial_packets[input][highest_class].front( );

      if ( f->head ) { // Find first available VC

	if ( _voqing ) {
	  if ( _buf_states[input]->IsAvailableFor( f->dest ) ) {
	    f->vc = f->dest;
	  }
	} else {
	  f->vc = _buf_states[input]->FindAvailable( );
	}
	  
	if ( f->vc != -1 ) {
	  _buf_states[input]->TakeBuffer( f->vc );
	}
      }

      if ( ( f->vc != -1 ) &&
	   ( !_buf_states[input]->IsFullFor( f->vc ) ) ) {

	_partial_packets[input][highest_class].pop_front( );
	_buf_states[input]->SendingFlit( f );
	write_flit = true;

	// Pass VC "back"
	if ( !_partial_packets[input][highest_class].empty( ) && !f->tail ) {
	  nf = _partial_packets[input][highest_class].front( );
	  nf->vc = f->vc;
	}
      }
    }

    _net->WriteFlit( write_flit ? f : 0, input );
  }

  _net->ReadInputs( );

  _partial_internal_cycles += _internal_speedup;
  while( _partial_internal_cycles >= 1.0 ) {
    _net->InternalStep( );
    _partial_internal_cycles -= 1.0;
  }

  _net->WriteOutputs( );

  ++_time;

  // Eject traffic and send credits

  for ( int output = 0; output < _net->NumDests( ); ++output ) {
    f = _net->ReadFlit( output );

    if ( f ) {
      if ( f->watch ) {
	cout << "ejected flit " << f->id << " at output " << output << endl;
	cout << "sending credit for " << f->vc << endl;
      }
      
      cred = new Credit( 1 );
      cred->vc[0] = f->vc;
      cred->vc_cnt = 1;

      _net->WriteCredit( cred, output );
      _RetireFlit( f, output );
      
      _accepted_packets[output]->AddSample( 1 );
    } else {
      _net->WriteCredit( 0, output );
      _accepted_packets[output]->AddSample( 0 );
    }
  }
}
  
bool TrafficManager::_PacketsOutstanding( ) const
{
  bool outstanding;

  if ( _measured_in_flight == 0 ) {
    outstanding = false;

    for ( int c = 0; c < _classes; ++c ) {
      for ( int s = 0; s < _sources; ++s ) {
	if ( !_qdrained[s][c] ) {
#ifdef DEBUG_DRAIN
	  cout << "waiting on queue " << s << " class " << c;
	  cout << ", time = " << _time << " qtime = " << _qtime[s][c] << endl;
#endif DEBUG_DRAIN
	  outstanding = true;
	  break;
	}
      }
      if ( outstanding ) { break; }
    }
  } else {
#ifdef DEBUG_DRAIN
    cout << "in flight = " << _measured_in_flight << endl;
#endif DEBUG_DRAIN
    outstanding = true;
  }

  return outstanding;
}

void TrafficManager::_ClearStats( )
{
  for ( int c = 0; c < _classes; ++c ) {
    _latency_stats[c]->Clear( );
  }
  
  for ( int i = 0; i < _dests; ++i ) {
    _accepted_packets[i]->Clear( );
    _pair_latency[i]->Clear( );
  }
}

int TrafficManager::_ComputeAccepted( double *avg, double *min ) const 
{
  int dmin;

  *min = 1.0;
  *avg = 0.0;

  for ( int d = 0; d < _dests; ++d ) {
    if ( _accepted_packets[d]->Average( ) < *min ) {
      *min = _accepted_packets[d]->Average( );
      dmin = d;
    }
    *avg += _accepted_packets[d]->Average( );
  }

  *avg /= (double)_dests;

  return dmin;
}

void TrafficManager::_DisplayRemaining( ) const 
{
  map<int, bool>::const_iterator iter;

  cout << "Remaining flits: ";
  for ( iter = _in_flight.begin( );
	iter != _in_flight.end( );
	iter++ ) {
    cout << iter->first << " ";
  }
  cout << endl;
}

bool TrafficManager::_SingleSim( )
{
  int  iter;
  int  total_phases;
  int  converged;
  int  max_outstanding;
  int  empty_steps;
  
  double cur_latency;
  double prev_latency;

  double cur_accepted;
  double prev_accepted;

  double warmup_threshold;
  double stopping_threshold;
  double acc_stopping_threshold;

  double min, avg;

  bool   clear_last;

  _time = 0;
  for ( int s = 0; s < _sources; ++s ) {
    for ( int c = 0; c < _classes; ++c  ) {
      _qtime[s][c]    = 0;
      _qdrained[s][c] = false;
    }
  }

  stopping_threshold     = 0.01;
  acc_stopping_threshold = 0.01;
  warmup_threshold       = 0.05;
  iter            = 0;
  converged       = 0;
  max_outstanding = 0;
  total_phases    = 0;

  // warm-up ...
  // reset stats, all packets after warmup_time marked
  // converge
  // draing, wait until all packets finish

  _sim_state    = warming_up;
  total_phases  = 0;
  prev_latency  = 0;
  prev_accepted = 0;

  _ClearStats( );
  clear_last    = false;

  while( ( total_phases < _max_samples ) && 
	 ( ( _sim_state != running ) || 
	   ( converged < 3 ) ) ) {

    if ( clear_last || ( ( _sim_state == warming_up ) && ( total_phases & 0x1 == 0 ) ) ) {
      clear_last = false;
      _ClearStats( );
    }

    for ( iter = 0; iter < _sample_period; ++iter ) { _Step( ); } 

    cout << "%=================================" << endl;

    int dmin;

    cur_latency = _latency_stats[0]->Average( );
    dmin = _ComputeAccepted( &avg, &min );
    cur_accepted = avg;

    cout << "% Average latency = " << cur_latency << endl;
    cout << "% Accepted packets = " << min << " at node " << dmin << " (avg = " << avg << ")" << endl;

    cout << "lat(" << total_phases + 1 << ") = " << cur_latency << ";" << endl;
    cout << "thru(" << total_phases + 1 << ",:) = [ ";
    for ( int d = 0; d < _dests; ++d ) {
      cout << _accepted_packets[d]->Average( ) << " ";
    }
    cout << "];" << endl;

    // Fail safe
    if ( ( _sim_mode == latency ) && ( cur_latency >_latency_thres ) ) {
      cout << "Average latency is getting huge" << endl;
      converged = 0; 
      _sim_state = warming_up;
      break;
    }

    cout << "% latency change    = " << fabs( ( cur_latency - prev_latency ) / cur_latency ) << endl;
    cout << "% throughput change = " << fabs( ( cur_accepted - prev_accepted ) / cur_accepted ) << endl;

    if ( _sim_state == warming_up ) {

      if ( _warmup_periods == 0 ) {
	if ( _sim_mode == latency ) {
	  if ( ( fabs( ( cur_latency - prev_latency ) / cur_latency ) < warmup_threshold ) &&
	       ( fabs( ( cur_accepted - prev_accepted ) / cur_accepted ) < warmup_threshold ) ) {
	    cout << "% Warmed up ..." << endl;
	    clear_last = true;
	    _sim_state = running;
	  }
	} else {
	  if ( fabs( ( cur_accepted - prev_accepted ) / cur_accepted ) < warmup_threshold ) {
	    cout << "% Warmed up ..." << endl;
	    clear_last = true;
	    _sim_state = running;
	  }
	}
      } else {
	if ( total_phases + 1 >= _warmup_periods ) {
	  cout << "% Warmed up ..." << endl;
	  clear_last = true;
	  _sim_state = running;
	}
      }
    } else if ( _sim_state == running ) {
      if ( _sim_mode == latency ) {
	if ( ( fabs( ( cur_latency - prev_latency ) / cur_latency ) < stopping_threshold ) &&
	     ( fabs( ( cur_accepted - prev_accepted ) / cur_accepted ) < acc_stopping_threshold ) ) {
	  ++converged;
	} else {
	  converged = 0;
	}
      } else {
	if ( fabs( ( cur_accepted - prev_accepted ) / cur_accepted ) < acc_stopping_threshold ) {
	    //++converged;
	} else {
	  converged = 0;
	  }
      } 
    }

    /*for ( int d = 0; d < _dests; ++d ) {
      if ( _sim_state == running ) {
	cout << "a(" << d+1 << "," << total_phases+1 << ") = " 
	<< _accepted_packets[d]->Average( ) << "; ";
      }
    }
    cout << endl;
    cout << "overall(" << total_phases+1 << ") = " << cur_accepted << endl;*/

    prev_latency  = cur_latency;
    prev_accepted = cur_accepted;

    ++total_phases;
  }

  if ( _sim_state == running ) {
    ++converged;

    if ( _sim_mode == latency ) {
      cout << "% Draining all recorded packets ..." << endl;
      _sim_state  = draining;
      _drain_time = _time;
      empty_steps = 0;
      while( _PacketsOutstanding( ) ) { 
	_Step( ); 
	++empty_steps;
	
	if ( empty_steps % 1000 == 0 ) {
	  _DisplayRemaining( ); 
	}
      }
    }
  } else {
    cout << "Too many sample periods needed to converge" << endl;
  }

  // Empty any remaining packets
  cout << "% Draining remaining packets ..." << endl;
  _empty_network = true;
  empty_steps = 0;
  while( _total_in_flight > 0 ) { 
    _Step( ); 
    ++empty_steps;

    if ( empty_steps % 1000 == 0 ) {
      _DisplayRemaining( ); 
    }
  }
  _empty_network = false;

  return ( converged > 0 );
}

void TrafficManager::Run( )
{
  double min, avg;

  _FirstStep( );

  for ( int sim = 0; sim < _total_sims; ++sim ) {
    if ( !_SingleSim( ) ) {
      cout << "Simulation unstable, ending ..." << endl;
      return;
    }
    
    for ( int c = 0; c < _classes; ++c ) {
      _overall_latency[c]->AddSample( _latency_stats[c]->Average( ) );
    }
    
    _ComputeAccepted( &avg, &min );
    _overall_accepted->AddSample( avg );
    _overall_accepted_min->AddSample( min );
  }

  for ( int c = 0; c < _classes; ++c ) {
    cout << "====== Traffic class " << c << " ======" << endl;

    cout << "Overall average latency = " << _overall_latency[c]->Average( )
	 << " (" << _overall_latency[c]->NumSamples( ) << " samples)" << endl;

    cout << "Overall average accepted rate = " << _overall_accepted->Average( )
       << " (" << _overall_accepted->NumSamples( ) << " samples)" << endl;

    cout << "Overall min accepted rate = " << _overall_accepted_min->Average( )
	 << " (" << _overall_accepted_min->NumSamples( ) << " samples)" << endl;

    //_latency_stats[c]->Display( );
  }

  cout << "Average hops = " << _hop_stats->Average( )
       << " (" << _hop_stats->NumSamples( ) << " samples)" << endl;

  //_hop_stats->Display( );
    
  /*for ( int i = 0; i < _dests; ++i ) {
    cout << "  Average to " << i << " = " << _pair_latency[i]->Average( ) << "( " 
	 << _pair_latency[i]->NumSamples( ) << " samples)" << endl;
    _pair_latency[i]->Display( );
    }*/
  
  //_net->Display( );

  for ( int i = 0 ; i < _net->NumSources( ); ++i ) {
    //cout << "input queue " << i << "'s length = " << _partial_packets[i].size( ) << endl;
    //_buf_states[i]->Display( );
  }
}
