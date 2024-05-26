#ifndef MCU_ADAPTER_H
#define MCU_ADAPTER_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "adapter.h"

#define _DEBUG_INFO (1)

//#if _ENABLE_MCU 
//#endif

class Mcu_adapter : public Adapter 
{
public:
  Mcu_adapter();
  ~Mcu_adapter();

  static int getFirewareMajor( char* _buf,int _size ) ;
  static int getFirewareMinor( char* _buf,int _size ) ;

  static int getHardwareMajor( char* _buf,int _size ) ;
  static int getHardwareMinor( char* _buf,int _size ) ;

  static int getAccVoltage( char* _buf,int _size ) ;
  static int getBatVoltage( char* _buf,int _size ) ;
private:
  static int m_fireware_major;
  static int m_fireware_minor;
  static int m_hardware_major;
  static int m_hardware_minor;
};
#endif 
