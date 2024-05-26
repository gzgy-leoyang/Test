#include "adapter/rtc_adapter.h"
#include "service/udsManager.h"
#include "support/debug_out.h"
#include "support/yq_rtc.h"
#include "main.h"

#include <iostream>
#include <fstream>

using std::ifstream;
using std::ios;
static struct rtc_time m_rtc_time;

Rtc_adapter::
Rtc_adapter():Adapter()
{
  memset( &m_rtc_time,0,sizeof( struct rtc_time ) );
  if ( get_rtc( &m_rtc_time ) < 0) {
    LOGW<<" Failed to get restart from file";
  } else {
    LOGI<<" Date & Time < "
        <<""<<m_rtc_time.tm_year
        <<"-"<<m_rtc_time.tm_mon
        <<"-"<<m_rtc_time.tm_mday
        <<" "<<m_rtc_time.tm_hour
        <<":"<<m_rtc_time.tm_min
        <<":"<<m_rtc_time.tm_sec<<">";
  }

  UdsManager* um = UdsManager::getInstance();
  UDS_operator_t op;
  op.safety   = 0;
  op.session  = 0;

  op.callback = getYear;
  um->registeGeneralOperatorWithDid(0x8900,op);

  op.callback = getMonth;
  um->registeGeneralOperatorWithDid(0x8901,op);

  op.callback = getDay;
  um->registeGeneralOperatorWithDid(0x8902,op);

  op.callback = getHour;
  um->registeGeneralOperatorWithDid(0x8903,op);

  op.callback = getMinute;
  um->registeGeneralOperatorWithDid(0x8904,op);

  op.callback = getSecond;
  um->registeGeneralOperatorWithDid(0x8905,op);
}
int Rtc_adapter::
getHour( char* _buf,int _size )
{
  UNUSED(_size);
  if ( m_rtc_time.tm_year== 0 ) {
    if ( get_rtc( &m_rtc_time ) < 0) {
      LOGW<<" Failed to get restart from file";
    }
  }
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = m_rtc_time.tm_hour;
  return 1;
}

int Rtc_adapter::
getMinute( char* _buf,int _size )
{
  UNUSED(_size);
  if ( m_rtc_time.tm_year== 0 ) {
    if ( get_rtc( &m_rtc_time ) < 0) {
      LOGW<<" Failed to get restart from file";
    }
  }
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = m_rtc_time.tm_min;
  return 1;
}

int Rtc_adapter::
getSecond( char* _buf,int _size )
{
  UNUSED(_size);
  if ( get_rtc( &m_rtc_time ) < 0) {
    LOGW<<" Failed to get restart from file";
  }
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = m_rtc_time.tm_sec;
  return 1;
}

int Rtc_adapter::getYear( char* _buf,int _size )
{
  UNUSED(_size);
  if ( m_rtc_time.tm_year== 0 ) {
    if ( get_rtc( &m_rtc_time ) < 0) {
      LOGW<<" Failed to get restart from file";
    }
  }
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = m_rtc_time.tm_year - 100;
  return 1;
}

int Rtc_adapter::getMonth( char* _buf,int _size )
{
  UNUSED(_size);
  if ( m_rtc_time.tm_year == 0 ) {
    if ( get_rtc( &m_rtc_time ) < 0) {
      LOGW<<" Failed to get restart from file";
    }
  }
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = m_rtc_time.tm_mon+1;
  return 1;
}

int Rtc_adapter::getDay( char* _buf,int _size )
{
  UNUSED(_size);
  if ( m_rtc_time.tm_year== 0 ) {
    if ( get_rtc( &m_rtc_time ) < 0) {
      LOGW<<" Failed to get restart from file";
    }
  }
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = m_rtc_time.tm_mday;
  return 1;
}
/// end of code ///
