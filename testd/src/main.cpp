#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>

#include "support/debug_out.h"
#include "support/c_debug_out.h"
#include "support/utils.h"
#include "support/yq_rtc.h"

#include "service/udsManager.h"
#include "net/net_can.h"
#include "event_handler.h"

#include "adapter/sys_adapter.h"
#include "adapter/device_adapter.h"
#include "adapter/hyman_adapter.h"
#include "adapter/orchid_adapter.h"
#include "adapter/launcher_adapter.h"
#include "adapter/kemp_adapter.h"
#include "adapter/mcu_adapter.h"
#include "adapter/lcd_adapter.h"
#include "adapter/rtc_adapter.h"

#include "mcu_ctrl.h"

#define _AUTHOR ("yangjing@yqyd.com.cn")
#define _MAJOR  (1)
#define _MINOR  (0)

enum {ERR=1};
static char m_this_address   = 0x01;
static char m_tester_address = 0x00;
static int  m_debug_level    = 0;

// 通过参数指定用于UDS的CAN端口
static char  m_uds_port[32]   = {"can0"};

Sys_adapter*      sys      = nullptr;
Orchid_adapter*   orchid   = nullptr;
Hyman_adapter*    hyman    = nullptr;
Launcher_adapter* launcher = nullptr;
Kemp_adapter*     kemp     = nullptr;
Mcu_adapter*      mcu      = nullptr;
Lcd_adapter*      lcd      = nullptr;
Rtc_adapter*      rtc      = nullptr;

void usage(void);
void prase_args ( int argc,char* argv[] );

int main ( int argc,char* argv[] )
{ 
  char cdt[20] = {0};
  if (  compile_datetime( cdt ) == 0) {
    char ver[64] = {0};
    snprintf( ver,50,"%i.%i.%s",
              _MAJOR,
              _MINOR,
              cdt );
    printf(" yqsrv_testd %s (yangjing@yqyd.com.cn)\n",ver);
  }


  prase_args ( argc,argv );
  // 日志的文件接口
  static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(_DEBUG_LOG_FILE,10240,3);
  // 日志的终端接口
  static plog::ColorConsoleAppender<plog::TxtFormatter> colorConsoleAppender;
  // 初始化plog,包含两个接口
  switch ( m_debug_level ) {
    case 0:
      plog::init( plog::error,&fileAppender).addAppender(&colorConsoleAppender);
      break;
    case 1:
      plog::init( plog::warning,&fileAppender).addAppender(&colorConsoleAppender);
      break;
    case 2:
      plog::init(plog::info,&fileAppender).addAppender(&colorConsoleAppender);
      break;
    case 3:
      plog::init(plog::verbose,&fileAppender).addAppender(&colorConsoleAppender);
      break;
  }
  // 添加第二个plog对象,用于记录错误消息
  plog::init<ERR>(plog::error, _ERR_LOG_FILE);

  // C 部分代码打印级
  CLOG_SET_LEVEL( _VERBOSE );

  /// 用于UDS测试接口
  Net_can* net_dev = new Net_can( m_this_address,
                                  m_tester_address, 
                                  m_uds_port,0);
  // 接收ID,来自Tester,所以
  // 组合地址= xx xx (DA)this_addr (SA)tester_addr
  int addr = m_this_address ;
  addr     = (addr << 8) | m_tester_address;
  LOGI << " DesAddr_SrcAddr: 0x"<<std::hex<<addr;

  // 物理寻址ID
  net_dev->register_callback_for_ID(0x0CEF0000|addr,
                                    event_0CEF_UDS);
  // 功能寻址ID,用于广播
  net_dev->register_callback_for_ID(0x0CEE0000|addr,
                                    event_0CEE_UDS);

  UdsManager* um = UdsManager::getInstance();
  um->addNetDev( net_dev );
  um->setLocalAddress( m_this_address );
  um->setTesterAddress( m_tester_address );

  sys      = new Sys_adapter();
  orchid   = new Orchid_adapter();
  hyman    = new Hyman_adapter();
  launcher = new Launcher_adapter();
  kemp     = new Kemp_adapter();
  mcu      = new Mcu_adapter();
  lcd      = new Lcd_adapter();
  rtc      = new Rtc_adapter();

  while( true ) {
    sleep(1);
  }
  return 0;
}

/////////////////////////////////////////////////
void prase_args ( int argc,char* argv[] )
{
  if ( argv == NULL ){
    usage();
    return;
  }

  int   opt = 0;
  while ((opt =  getopt(argc, argv,"T:S:d:p:h")) != -1) {
    switch(opt) {
      case 'T':
        m_tester_address = atoi(optarg);
        break;
      case 'S':
        m_this_address = atoi(optarg);
        break;
      case 'd':
        m_debug_level = atoi(optarg);
        break;
      case 'p':
        strcpy(m_uds_port,optarg);
        break;
      case 'h':
        usage();
        exit(0);
        break;
      default:
        usage();
        exit(0);
        break;
    }
  }
}

void usage(void)
{
  printf("Usage: spy [-h] [-T] [-S] [-d]\n");
  printf("Option:\n");
  printf(" -h Help\n");
  printf(" -T Tester address ,default:0\n");
  printf(" -S Device tested address,default:1\n");
  printf(" -p CAN Port used by UDS,default:can0\n");
  printf(" -d Debug level,Default:0(error) 1:warning 2:info 3:detail\n");
}
// end of code //
