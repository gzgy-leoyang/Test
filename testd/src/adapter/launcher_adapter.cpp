#include "adapter/launcher_adapter.h"
#include "support/debug_out.h"
#include "service/udsManager.h"
#include "main.h"

#include <iostream>
#include <fstream>

using std::ifstream;
using std::ios;

// 记录Launcher状态
Launcher_state_t Launcher_adapter::m_state;

Launcher_adapter::
Launcher_adapter():Adapter()
{
  std::memset( &m_state,0,sizeof( Launcher_state_t ) );

  UdsManager* um = UdsManager::getInstance();

  UDS_operator_t op;
  op.safety=0;
  op.session=0;
  op.callback=getRestartCounter;
  um->registeGeneralOperatorWithDid(0x8301,op);

  op.callback=getHeartbeat;
  um->registeGeneralOperatorWithDid(0x8302,op);
}

int Launcher_adapter::
getRestartCounter( char* _buf,int _size )
{
  UNUSED(_size);
  if ( readBinaryFromFile( "/run/launcher/state",
                          reinterpret_cast<void*>(&m_state) ,
                          sizeof( Launcher_state_t )) < 0 ) {
    LOGW<<" Failed to get restart from file";
    return 0;
  }
  LOGI<<" Launcher Restart: "<< static_cast<int>(m_state.restart)  ;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = m_state.restart;
  return 1;
}

int Launcher_adapter::
getHeartbeat( char* _buf,int _size )
{
  UNUSED(_size);
  if ( readBinaryFromFile( "/run/launcher/state",
                          reinterpret_cast<void*>(&m_state) ,
                          sizeof( Launcher_state_t )) < 0 ) {
    LOGW<<" Failed to get restart from file";
    return 0;
  }
  LOGI<<" Launcher Heartbeat: "<<static_cast<int>(m_state.heartbeat);
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = m_state.heartbeat;
  return 1;
}
/// end of code ///
