#ifndef _CREDIT_HPP_
#define _CREDIT_HPP_

class Credit {
public:
  Credit( int max_vcs = 1 );
  ~Credit( );

  int *vc;
  int vc_cnt;
};

#endif _CREDIT_HPP_
