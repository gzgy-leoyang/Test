#ifndef LAUNCHER_ADAPTER_H
#define LAUNCHER_ADAPTER_H

#include "adapter.h"
#include <stdint.h>

typedef struct {
  uint8_t restart;
  uint8_t heartbeat;
} Launcher_state_t;

class Launcher_adapter : public Adapter 
{

public:
  Launcher_adapter();
  ~Launcher_adapter();

  static Launcher_state_t m_state;
  static int getRestartCounter( char* _buf,int _size ) ;
  static int getHeartbeat( char* _buf,int _size ) ;
};
#endif
