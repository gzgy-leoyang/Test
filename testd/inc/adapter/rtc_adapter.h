#ifndef RTC_ADAPTER_H
#define RTC_ADAPTER_H

#include "adapter.h"
#include <stdint.h>

class Rtc_adapter : public Adapter 
{
public:
  Rtc_adapter();
  ~Rtc_adapter();

  static int getHour( char* _buf,int _size );
  static int getMinute( char* _buf,int _size );
  static int getSecond( char* _buf,int _size );
  static int getYear( char* _buf,int _size );
  static int getMonth( char* _buf,int _size );
  static int getDay( char* _buf,int _size );
};
#endif
