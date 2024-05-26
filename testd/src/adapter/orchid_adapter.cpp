#include "adapter/orchid_adapter.h"
#include "support/debug_out.h"
#include "service/udsManager.h"
#include "main.h"

#include <iostream>
#include <fstream>

using std::ifstream;
using std::ios;

Orchid_adapter::
Orchid_adapter():Adapter()
{
  UdsManager* um = UdsManager::getInstance();

  UDS_operator_t op;
  op.safety=0;
  op.session=0;
  op.callback=getIgnition;
  um->registeGeneralOperatorWithDid(0x8201,op);
}

int Orchid_adapter::
getIgnition( char* _buf,int _size )
{
  UNUSED(_size);

  int ignition_sta = 0;
  if ( readIntFromFile( "/run/orchid/ignition",
                        ignition_sta) < 0 ) {
    LOGW<<" Failed to get ignition from file";
    return 0;
  }
  LOGI<<" Ignition: "<<ignition_sta;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ignition_sta ;
  return 1;
}
