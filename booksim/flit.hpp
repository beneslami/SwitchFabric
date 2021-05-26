#ifndef _FLIT_HPP_
#define _FLIT_HPP_

#include <iostream>

#ifdef _WIN32_
#include <ostream>
#endif _WIN32_

struct Flit {
  int vc;

  bool head;
  bool tail;
  
  int  time;
  int  id;
  bool record;

  int  src;
  int  dest;

  int  pri;

  int  hops;
  bool watch;

  // Fields for multi-phase algorithms
  mutable int intm;
  mutable int ph;

  mutable int dr;

  // Which VC parition to use for deadlock avoidance in a ring
  mutable int ring_par;
};

ostream& operator<<( ostream& os, const Flit& f );

#endif _FLIT_HPP_
