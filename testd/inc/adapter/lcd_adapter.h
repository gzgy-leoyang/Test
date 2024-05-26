#ifndef LCD_ADAPTER_H
#define LCD_ADAPTER_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "adapter.h"

class Lcd_adapter : public Adapter 
{
public:
  Lcd_adapter();
  ~Lcd_adapter();

  int getPixel(const int _x,const int _y);

  static int init( char* _drm_dev );

  static int m_piexl_x;
  static int m_piexl_y;

  static int setPixelX( const int _x);
  static int setPixelY( const int _y);
  static int getPixelColor(int* _r,int* _g,int* _b);

  static int getColor( char* _buf,int _size ) ;
  static int piexlX_rw( char* _buf,int _size ) ;
  static int piexlY_rw( char* _buf,int _size ) ;
private:
  
};
  #endif 
