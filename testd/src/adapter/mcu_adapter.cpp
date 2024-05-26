#include "adapter/mcu_adapter.h"

#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "support/sys_info.h"
#include "adapter/adapter.h"
#include "plog/Log.h"

#include "service/udsManager.h"
#include "main.h"

// _RK3568_MCU 宏定义来自 src/CMakeLists.txt中,根据芯片平台类型定义添加
#ifdef _RK3568_MCU 
  #include "mcu_ctrl.h"
#endif

int Mcu_adapter::m_fireware_major = 0;
int Mcu_adapter::m_fireware_minor = 0;
int Mcu_adapter::m_hardware_major = 0;
int Mcu_adapter::m_hardware_minor = 0;

Mcu_adapter::
Mcu_adapter() : Adapter()
{
#ifdef _RK3568_MCU 
  uart_set_uart_dev( (char*)"/dev/ttyS8" );
  if ( uart_init() < 0) {
    LOGE << " Failed to initilize UART";
  }

  int br = 0;
  if ( uart_get_bitrate( &br ) <0) {
    LOGE << " Failed to get bitrate of UART";
  }
  LOGI << " MCU Uart bitrate:"<<br;

  char buf[128];
  if ( uart_get_fireware_version( buf ) <0 ) {
    LOGW << " Failed to get MCU Fireware version";
  } else {
    sscanf( buf,"YQ0.289.%iV%i", 
            &m_fireware_major,
            &m_fireware_minor);
    LOGI << " MCU Fireware version:"<<m_fireware_major<<"."<<m_fireware_minor;
  }
  memset( buf,0,128 );

  if ( uart_get_hardware_version( buf ) <0 ) {
    LOGW << " Failed to get MCU Hardware version";
  } else {
    sscanf( buf,"V%i", &m_hardware_major);
    m_hardware_minor = m_hardware_major;
    LOGI << " MCU Hardware version:"<<m_hardware_major<<"."<<m_hardware_minor;
  }
#endif


  if ( uart_set_gpio(9,1) < 0) {
    LOGW << " Failed to set MCU GPIO";
  } else {
    LOGI << " Set PC15 high...OK";
  }

  UdsManager* um = UdsManager::getInstance();

  UDS_operator_t op;
  op.safety   = 0;
  op.session  = 0;
  op.callback = getFirewareMajor;
  um->registeGeneralOperatorWithDid(0x8700,op);

  op.callback = getFirewareMinor;
  um->registeGeneralOperatorWithDid(0x8701,op);

  op.callback = getHardwareMajor;
  um->registeGeneralOperatorWithDid(0x8702,op);

  op.callback = getHardwareMinor;
  um->registeGeneralOperatorWithDid(0x8703,op);

  op.callback = getAccVoltage;
  um->registeGeneralOperatorWithDid(0x8704,op);

  op.callback = getBatVoltage;
  um->registeGeneralOperatorWithDid(0x8705,op);
}

Mcu_adapter::
~Mcu_adapter()
{
}


int Mcu_adapter::
getFirewareMajor( char* _buf,int _size )
{
  int v1 = 0;
  int v2 = 0;
  UNUSED(_size);
#ifdef _RK3568_MCU 
  if ( m_fireware_major != 0 ) {
    v1 = m_fireware_major;
  } else {
    char buf[128];
    if ( uart_get_fireware_version( buf ) <0 ) {
    } else {
      sscanf( buf,"YQ0.289.%iV%i", &v1,&v2);
      m_fireware_major = v1;
      LOGI << " MCU Fireware version:"<<buf<<" V1="<<v1<<" V2="<<v2;
    }
  }
#endif
  LOGI << " MCU Fireware major:"<<m_fireware_major;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = (v1 & 0xFF00) >> 8;
  *(_buf+5) = (v1 & 0x00FF) ;
  return 2;
}

int Mcu_adapter::
getFirewareMinor( char* _buf,int _size )
{
  int v1 = 0;
  int v2 = 0;
  UNUSED(_size);
#ifdef _RK3568_MCU 
  if ( m_fireware_minor != 0 ) {
    v2 = m_fireware_minor;
  } else {
    char buf[128];
    if ( uart_get_fireware_version( buf ) <0 ) {
      LOGW << " Failed to get MCU Fireware version";
      return -1;
    } else {
      sscanf( buf,"YQ0.289.%iV%i", &v1,&v2);
      m_fireware_minor = v2;
      LOGI << " MCU Fireware version:"<<buf<<" V1="<<v1<<" V2="<<v2;
    }
  }
#endif
  LOGI << " MCU Fireware minor:"<<m_fireware_minor;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = (v2 & 0xFF00) >> 8;
  *(_buf+5) = (v2 & 0x00FF) ;
  return 2;
}

int Mcu_adapter::
getHardwareMajor( char* _buf,int _size )
{
  int v = 0;
  UNUSED(_size);
#ifdef _RK3568_MCU 
  if ( m_hardware_major != 0 ) {
    v = m_hardware_major;
  } else {
    char buf[128];
    if ( uart_get_hardware_version( buf ) <0 ) {
      LOGW << " Failed to get MCU Hardware version";
      return -1;
    } else {
      sscanf( buf,"V%i", &v);
      m_hardware_major = v ;
    }
  }
#endif
  LOGI << " MCU Hardware major:"<<m_hardware_major;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = (v & 0xFF00) >> 8;
  *(_buf+5) = (v & 0x00FF) ;
  return 2;
}

int Mcu_adapter::
getHardwareMinor( char* _buf,int _size )
{
  UNUSED(_size);
  int v = 0;
#ifdef _RK3568_MCU 
  if ( m_hardware_minor!= 0 ) {
    v = m_hardware_minor;
  } else  {
    char buf[128];
    memset( buf,0,128 );
    if ( uart_get_hardware_version( buf ) <0 ) {
      LOGW << " Failed to get MCU Hardware version";
      return -1;
    } else {
      sscanf( buf,"V%i", &v);
      m_hardware_minor = v;
      LOGI << " MCU Hardware version:"<<buf<<" V="<<v;
    }
  }
#endif
  LOGI << " MCU Hardware minor:"<<m_hardware_minor;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = (v & 0xFF00) >> 8;
  *(_buf+5) = (v & 0x00FF) ;
  return 2;
}

int Mcu_adapter::
getAccVoltage( char* _buf,int _size )
{
  UNUSED(_size);
#ifdef _RK3568_MCU 
  int acc = 0;
  int bat = 0;

  int retry = 0;
  int ret  = 0;
  do {
    ret = uart_get_acc_bat_voltage( &acc, &bat);
    if ( ret <0 ) {
      retry++;
      usleep(5000*retry);
    }
  } while ((ret<0) && (retry<5));

  if ( (ret<0)&&(retry>=5) ) {
    LOGW << " Failed to get ACC voltage";
  } else {
    LOGI << " MCU ACC Voltage:"<<acc;
    std::memset((_buf+4),0xAA,4);
    *(_buf+4) = ( acc & 0xFF00) >> 8;
    *(_buf+5) = ( acc & 0x00FF) ;
    return 2;
  }
#endif
  return -1;
}

int Mcu_adapter::
getBatVoltage( char* _buf,int _size )
{
  UNUSED(_size);
#ifdef _RK3568_MCU 
  int acc = 0;
  int bat = 0;
  int retry = 0;
  int ret  = 0;
  do {
    ret = uart_get_acc_bat_voltage( &acc, &bat);
    if ( ret <0 ) {
      retry++;
      usleep(5000*retry);
    }
  } while ((ret<0) && (retry<5));

  if ( (ret<0)&&(retry>=5) ) {
    LOGW << " Failed to get BAT voltage";
  } else {
    LOGI << " MCU BAT Voltage:"<<bat;
    std::memset((_buf+4),0xAA,4);
    *(_buf+4) = ( bat & 0xFF00) >> 8;
    *(_buf+5) = ( bat & 0x00FF) ;
    return 2;
  }
#endif
  return -1;
}

/// end of code ///
