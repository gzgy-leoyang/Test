#include "adapter/sys_adapter.h"

#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "support/sys_info.h"
//#include "support/c_debug_out.h"
#include "adapter/adapter.h"

#include "service/udsManager.h"
#include "main.h"

#include <climits>
#include <cstring>
#include <iostream>

Sys_adapter::
Sys_adapter() : Adapter()
{
  UdsManager* um = UdsManager::getInstance();

  UDS_operator_t op;
  op.safety=0;
  op.session=0;
  op.callback=getCpuID;
  um->registeGeneralOperatorWithDid(0x8100,op);

  op.callback=getUptime;
  um->registeGeneralOperatorWithDid(0x8101,op);

  op.callback=getLoadAvg_1;
  um->registeGeneralOperatorWithDid(0x8110,op);

  op.callback=getLoadAvg_5;
  um->registeGeneralOperatorWithDid(0x8111,op);

  op.callback=getLoadAvg_15;
  um->registeGeneralOperatorWithDid(0x8112,op);

  op.callback=getCpuFreq;
  um->registeGeneralOperatorWithDid(0x8120,op);

  op.callback=getCpuFreq1;
  um->registeGeneralOperatorWithDid(0x8121,op);

  op.callback=getCpuFreq2;
  um->registeGeneralOperatorWithDid(0x8122,op);

  op.callback=getCpuFreq3;
  um->registeGeneralOperatorWithDid(0x8123,op);

  op.callback=getTotalMem;
  um->registeGeneralOperatorWithDid(0x8130,op);

  op.callback=getAvailMem;
  um->registeGeneralOperatorWithDid(0x8131,op);

  op.callback=getCpuTemp;
  um->registeGeneralOperatorWithDid(0x8140,op);

  op.callback=getCpuTemp2;
  um->registeGeneralOperatorWithDid(0x8141,op);

  op.callback=brightness_rw;
  um->registeGeneralOperatorWithDid(0x8142,op);

  op.callback=getCan0Rx;
  um->registeGeneralOperatorWithDid(0x8150,op);

  op.callback=getCan0Tx;
  um->registeGeneralOperatorWithDid(0x8151,op);

}

Sys_adapter::
~Sys_adapter()
{
}


int Sys_adapter::
brightness_rw( char* _buf,int _size )
{
  int sid = *(_buf+1);
  int br = 0;
  if ( sid == 0x22 ) {
    std::string str;
    if(readStringFromFile("/sys/class/backlight/backlight3/brightness",
                          str)<0) {
      br = 255;
    } else {
      br = atoi(str.c_str());
    }
    LOGI<<" Get Brightness:"<<br;
    std::memset((_buf+4),0xAA,4);
    *(_buf+4) = (br& 0x00FF);
    return 1;
  } else if ( sid == 0x2E ) {
    int temp = *(_buf+4);
    temp     = ( temp << 8 ) | *(_buf+5);
    temp     = ( temp << 8 ) | *(_buf+6);
    temp     = ( temp << 8 ) | *(_buf+7);
    LOGI<<" Set Brightness:"<<temp;
    if ( temp > 256 )
      temp = 256;
    std::string str = std::to_string(temp);
    writeStringToFile("/sys/class/backlight/backlight3/brightness",
                      str) ;
    std::memset((_buf+4),0xAA,4);
    return 0;
  } else {
    // undefine sid
  }
  return 0;
}

int Sys_adapter::
getCpuID ( char* _buf,int _size ) 
{
  UNUSED(_size);

  long long unsigned int id = 0;
  if ( sysInfo_getCpuID( &id ) < 0 ) {
    LOGW<<" Failed to get CPU ID";
    return 0;
  }
  LOGI<<" CPU ID:"<<std::hex<<id;
  *(_buf+4) = (id& 0xFF000000) >> 24;
  *(_buf+5) = (id& 0x00FF0000) >> 16;
  *(_buf+6) = (id& 0x0000FF00) >> 8;
  *(_buf+7) = (id& 0x000000FF);
  return 4;
}

int Sys_adapter::
getUptime( char* _buf,int _size ) 
{
  UNUSED(_size);
  long uptime = 0;
  if ( sysInfo_getUptime( &uptime ) < 0 ) {
    LOGW<<" Failed to get uptime";
    return 0;
  }
  LOGI<<" Uptime:"<<uptime;
  *(_buf+4) = ( uptime & 0xFF000000) >> 24;
  *(_buf+5) = ( uptime & 0x00FF0000) >> 16;
  *(_buf+6) = ( uptime & 0x0000FF00) >> 8;
  *(_buf+7) = ( uptime & 0x000000FF);
  return 4;
}

/// 温度 
int Sys_adapter::
getCpuTemp( char* _buf,int _size )
{
  UNUSED(_size);
  int t1 = 0;
  int t2 = 0;
  if ( sysInfo_getTemp( &t1,&t2 ) < 0 ) {
    LOGW<<" Failed to get Cpu temp";
    return 0;
  }
  LOGI<<" T0:"<<t1;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = t1 ;
  return 1;
}

int Sys_adapter::
getCpuTemp2( char* _buf,int _size )
{
  UNUSED(_size);
  int t1 = 0;
  int t2 = 0;
  if ( sysInfo_getTemp( &t1,&t2 ) < 0 ) {
    LOGW<<" Failed to get Cpu temp2";
    return 0;
  }
  LOGI<<" T1:"<<t2;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = t2 ;
  return 1;
}
/// 完成: 温度 

/// 平均负载
int Sys_adapter::
getLoadAvg_1( char* _buf,int _size ) 
{
  UNUSED(_size);
  int load = 0;
  if ( sysInfo_getLoadAvg_1min( &load ) < 0 ) {
    load = 0;
    LOGW<<" Failed to get LoadAvg";
    return 0;
  }
  LOGI<<" Loadavg (1min):"<<load;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( load  & 0xFF00) >> 8;
  *(_buf+5) = ( load  & 0x00FF);
  return 2;
}

int Sys_adapter::
getLoadAvg_5( char* _buf,int _size )
{
  UNUSED(_size);
  int load = 0;
  if ( sysInfo_getLoadAvg_5min( &load ) < 0 ) {
    load = 0;
    LOGW<<" Failed to get LoadAvg";
    return 0;
  }
  LOGI<<" Loadavg (5min):"<<load;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( load  & 0xFF00) >> 8;
  *(_buf+5) = ( load  & 0x00FF);
  return 2;
}

int Sys_adapter::
getLoadAvg_15( char* _buf,int _size )
{
  UNUSED(_size);
  int load = 0;
  if ( sysInfo_getLoadAvg_15min( &load ) < 0 ) {
    load = 0;
    LOGW<<" Failed to get LoadAvg";
    return 0;
  }
  LOGI<<" Loadavg (15min):"<<load;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( load  & 0xFF00) >> 8;
  *(_buf+5) = ( load  & 0x00FF);
  return 2;
}
/// 完成:平均负载

/// CPU频率
int Sys_adapter::
getCpuFreq( char* _buf,int _size )
{
  UNUSED(_size);
  unsigned int freq = 0;
  if ( sysInfo_getCpuFreq( 0,&freq) < 0 ) {
    freq = 0;
    LOGW<<" Failed to get Cpu Freq";
    return 0;
  }
  LOGI<<" Cpu Freq:"<<freq;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( freq  & 0xFF00) >> 8;
  *(_buf+5) = ( freq  & 0x00FF);
  return 2;
}
int Sys_adapter::
getCpuFreq1( char* _buf,int _size )
{
  UNUSED(_size);
  unsigned int freq = 0;
  if ( sysInfo_getCpuFreq( 1,&freq) < 0 ) {
    freq = 0;
    LOGW<<" Failed to get Cpu Freq";
    return 0;
  }
  LOGI<<" Cpu Freq:"<<freq;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( freq  & 0xFF00) >> 8;
  *(_buf+5) = ( freq  & 0x00FF);
  return 2;
}
int Sys_adapter::
getCpuFreq2( char* _buf,int _size )
{
  UNUSED(_size);
  unsigned int freq = 0;
  if ( sysInfo_getCpuFreq( 2,&freq) < 0 ) {
    freq = 0;
    LOGW<<" Failed to get Cpu Freq";
    return 0;
  }
  LOGI<<" Cpu Freq:"<<freq;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( freq  & 0xFF00) >> 8;
  *(_buf+5) = ( freq  & 0x00FF);
  return 2;
}
int Sys_adapter::
getCpuFreq3( char* _buf,int _size )
{
  UNUSED(_size);
  unsigned int freq = 0;
  if ( sysInfo_getCpuFreq( 3,&freq) < 0 ) {
    freq = 0;
    LOGW<<" Failed to get Cpu Freq";
    return 0;
  }
  LOGI<<" Cpu Freq:"<<freq;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( freq  & 0xFF00) >> 8;
  *(_buf+5) = ( freq  & 0x00FF);
  return 2;
}
/// 完成:CPU频率

/// 内存
int Sys_adapter::
getTotalMem( char* _buf,int _size )
{
  UNUSED(_size);
  int total = 0;
  int avail = 0;
  if ( sysInfo_getMem( &total,&avail) < 0 ) {
    total = 0;
    avail = 0;
    LOGW<<" Failed to get Total Mem";
    return 0;
  }
  LOGI<<" Total Mem:"<<total;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( total & 0xFF000000) >> 24;
  *(_buf+5) = ( total & 0x00FF0000) >> 16;
  *(_buf+6) = ( total & 0x0000FF00) >> 8;
  *(_buf+7) = ( total & 0x000000FF);
  return 4;
}

int Sys_adapter::
getAvailMem( char* _buf,int _size )
{
  UNUSED(_size);
  int total = 0;
  int avail = 0;
  if ( sysInfo_getMem( &total,&avail) < 0 ) {
    total = 0;
    avail = 0;
    LOGW<<" Failed to get Avail Mem";
    return 0;
  }
  LOGI<<" Total Mem:"<<avail;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( avail  & 0xFF000000) >> 24;
  *(_buf+5) = ( avail  & 0x00FF0000) >> 16;
  *(_buf+6) = ( avail  & 0x0000FF00) >> 8;
  *(_buf+7) = ( avail  & 0x000000FF);
  return 4;
}
/// 完成: 内存

int Sys_adapter::
getCan0Rx( char* _buf,int _size )
{
  UNUSED(_size);
  int rx = 0;
  if ( sysInfo_getCanRx( 0,&rx) < 0 ) {
    rx = 0;
    LOGW<<" Failed to get CAN0 Rx";
    return 0;
  }
  LOGI<<" Can0 RX:"<<rx;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( rx & 0xFF000000) >> 24;
  *(_buf+5) = ( rx & 0x00FF0000) >> 16;
  *(_buf+6) = ( rx & 0x0000FF00) >> 8;
  *(_buf+7) = ( rx & 0x000000FF);
  return 4;
}

int Sys_adapter::
getCan0Tx( char* _buf,int _size )
{
  UNUSED(_size);
  int tx = 0;
  if ( sysInfo_getCanTx( 0,&tx) < 0 ) {
    tx = 0;
    LOGW<<" Failed to get CAN0 Tx";
    return 0;
  }
  LOGI<<" Can0 TX:"<<tx;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( tx & 0xFF000000) >> 24;
  *(_buf+5) = ( tx & 0x00FF0000) >> 16;
  *(_buf+6) = ( tx & 0x0000FF00) >> 8;
  *(_buf+7) = ( tx & 0x000000FF);
  return 4;
}

int Sys_adapter::
getCan1Rx( char* _buf,int _size )
{
  UNUSED(_buf);
  UNUSED(_size);
  return 0;
}

int Sys_adapter::
getCan1Tx( char* _buf,int _size )
{
  UNUSED(_buf);
  UNUSED(_size);
  return 0;
}
/// end of code ///
