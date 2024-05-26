#ifndef HYMAN_ADAPTER_H
#define HYMAN_ADAPTER_H

#include "adapter.h"

class Hyman_adapter : public Adapter 
{
public:
  Hyman_adapter();
  ~Hyman_adapter();

  static int getSecFromHyman_x( char* _buf,int _size ) ;
  static int getSecFromHyman( char* _buf,int _size ) ;
  static int getSecFromHourmeter( char* _buf,int _size ) ;
};

#endif
