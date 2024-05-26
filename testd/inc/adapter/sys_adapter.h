//#ifdef __cplusplus
//extern "C"{
//#endif

#ifndef SYS_ADAPTER_H
#define SYS_ADAPTER_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "adapter.h"

#define _DEBUG_INFO (1)


class Sys_adapter : public Adapter 
{
public:
  Sys_adapter();
  ~Sys_adapter();

  static int getCpuID( char* _buf,int _size ) ;
  static int getUptime( char* _buf,int _size ) ;
  static int getLoadAvg_1( char* _buf,int _size ) ;
  static int getLoadAvg_5( char* _buf,int _size ) ;
  static int getLoadAvg_15( char* _buf,int _size ) ;
  static int getCpuFreq( char* _buf,int _size ) ;
  static int getCpuFreq1( char* _buf,int _size ) ;
  static int getCpuFreq2( char* _buf,int _size ) ;
  static int getCpuFreq3( char* _buf,int _size ) ;
  static int getTotalMem( char* _buf,int _size ) ;
  static int getAvailMem( char* _buf,int _size ) ;
  static int getCpuTemp( char* _buf,int _size ) ;
  static int getCpuTemp2( char* _buf,int _size ) ;
  static int getCan0Rx( char* _buf,int _size ) ;
  static int getCan0Tx( char* _buf,int _size ) ;
  static int getCan1Rx( char* _buf,int _size ) ;
  static int getCan1Tx( char* _buf,int _size ) ;

  static int brightness_rw( char* _buf,int _size ) ;
private:
};
#endif 
