#ifndef DEVICE_ADAPTER_H
#define DEVICE_ADAPTER_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "adapter.h"

class Device_adapter : public Adapter 
{
public:
  Device_adapter();
  ~Device_adapter();

  static int setScreenWidth( char* _buf,int _size );
  static int setScreenHeight( char* _buf,int _size );
  static int setScreenColor( char* _buf,int _size );
  static int setScreenTpX( char* _buf,int _size );
  static int setScreenTpY( char* _buf,int _size );
private:
  static int m_scr_width  ;
  static int m_scr_height ;
  static int m_scr_color  ;
  static int m_tp_x       ;
  static int m_tp_y       ;
};
  #endif 
