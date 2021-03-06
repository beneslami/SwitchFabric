#ifndef _MODULE_HPP_
#define _MODULE_HPP_

#include <string>
#include <vector>

class Module {
protected:
  string _name;
  string _fullname;

  vector<Module *> _children;

  void _AddChild( Module *child );

public:
  Module( );
  Module( Module *parent, const string& name );
  virtual ~Module( ) { }
  
  void SetName( Module *parent, const string& name );

  void DisplayHierarchy( int level = 0 ) const;

  void Error( const string& msg );
  void Debug( const string& msg );

  virtual void Display( ) const;
};

#endif _MODULE_HPP_
