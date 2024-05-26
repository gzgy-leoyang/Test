#ifndef ORCHID_ADAPTER_H
#define ORCHID_ADAPTER_H

#include "adapter.h"

class Orchid_adapter : public Adapter 
{
public:
  Orchid_adapter();
  ~Orchid_adapter();

  static int getIgnition( char* _buf,int _size ) ;
};

#endif
