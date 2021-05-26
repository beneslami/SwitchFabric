
template<class T> class PipelineFIFO {
  int _lanes;
  int _depth;

public:
  PipelineFIFO( int lanes, int depth );
};

_c_fifo_ptr = 0;
  _c_fifo_len = _st_prepare_delay + _st_final_delay + 1;

  _crossbar_fifo = new Flit ** [_outputs];
  for ( int output = 0; output < _outputs; ++output ) {
    _crossbar_fifo[output] = new Flit * [_c_fifo_len];

    for ( int i = 0; i < _c_fifo_len; ++i ) {
      _crossbar_fifo[output][i] = 0;
    }
