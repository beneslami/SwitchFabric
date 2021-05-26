#ifndef _VC_ROUTER_HPP_
#define _VC_ROUTER_HPP_

#include <string>
#include <queue>

#include "module.hpp"
#include "router.hpp"
#include "vc.hpp"
#include "allocator.hpp"
#include "routefunc.hpp"
#include "selectfunc.hpp"
#include "outputset.hpp"
#include "buffer_state.hpp"
#include "pipefifo.hpp"

class VCRouter : public Router {
  int _vcs;
  int _vc_size;

  VC          **_vc;
  BufferState *_next_vcs;

  Allocator *_vc_allocator;
  Allocator *_sw_allocator;

  int *_sw_rr_offset;

  tRoutingFunction   _rf;
  tSelectionFunction _sf;

  OutputSet *_selected_set;

  PipelineFIFO<Flit>   *_crossbar_pipe;
  PipelineFIFO<Credit> *_credit_pipe;

  queue<Flit *> *_input_buffer;
  queue<Flit *> *_output_buffer;

  queue<Credit *> *_in_cred_buffer;
  queue<Credit *> *_out_cred_buffer;

  int _hold_switch_for_packet;
  int *_switch_hold_in;
  int *_switch_hold_out;
  int *_switch_hold_vc;

  void _ReceiveFlits( );
  void _ReceiveCredits( );

  void _InputQueuing( );
  void _Route( );
  void _VCAlloc( );
  void _SWAlloc( );
  void _OutputQueuing( );

  void _SendFlits( );
  void _SendCredits( );
  
  void _AddVCRequests( VC* cur_vc, int input_index, bool watch = false );

public:
  VCRouter( const Configuration& config,
	    Module *parent, string name, int id,
	    int inputs, int outputs );

  virtual ~VCRouter( );

  virtual void ReadInputs( );
  virtual void InternalStep( );
  virtual void WriteOutputs( );

  void Display( ) const;
};

#endif _VC_ROUTER_HPP_
