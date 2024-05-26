#include "tester.h"

#include <map>
#include <iterator>
#include <cstring>
#include <iostream>
#include <vector>

#include <fstream>
#include <sstream>

#include "main.h"

using namespace std;

int   init_test_sys();
void  receive_ack( struct can_frame* _frame );
void* thread_handler(void* arg);
void info( const char* _function,const char* _msg );

///////////////////////////////////////////////////////////
// 测试核心
// 首先是数据独立与Uds_tester模块,则对数据进行操作的函数自然
// 就可以独立出来,Uds_test模块现在仅用于启动一个线程,后续的工作
// 都会由线程运行,同时 Uds_test 模块还可以提供一个C-CPP的接口,
// 与其他模块进行交互

static void set_can_switch ( const int _io );

int  request_DID( const int _did ) ;

// **必须修改,添加测试动作定义**
/// 0. 电源控制
void poweron_battery(void);
void poweron_ignition(void);
void poweroff_ignition(void);
void poweroff_battery(void);

/// 1. System
void request_cpuid(void);
void request_uptime(void);
void request_core_tempeture_0(void);
void request_loadavg_1min(void);
void request_core_freq_0(void);
void request_avalid_mem(void);

/// 2. Orchid
void request_orchid_ignition(void);
void hook_orchid_ignition( struct can_frame* _f);

void request_hourmeter_zl_sec(void);
void request_hourmeter_lv_sec(void);
void request_hourmeter_sec(void);

///
void request_launcher_restart(void);
void hook_launcher_restart( struct can_frame* _f);
void request_launcher_heartbeat(void);

void request_kemp_pressed(void);
void request_kemp_release(void);

void request_mcu_fireware(void);
void request_mcu_fireware_minor(void);
void request_mcu_hardware(void);
void request_mcu_hardware_minor(void);
void request_mcu_acc_voltage(void);
void hook_mcu_acc_voltage( struct can_frame* _f );
void request_mcu_bat_voltage(void);

int rand_pwr_ctrl (void) ;

void after_poweroff(void) ;

int can_io = 0;

static std::vector< Time_active_t > customize_actives_queue ;


int insert_op( std::string& _op,int _msec,int _rep )
{
  std::map< std::string, Active_hook > name_activer_map = {
    {"POWER_ON_BAT"            , poweron_battery           },  
    {"POWER_ON_ACC"            , poweron_ignition          },
    {"POWER_OFF_ACC"           , poweroff_ignition         },
    {"POWER_OFF_BAT"           , poweroff_battery          },
    {"GET_CPU_ID"              , request_cpuid             },
    {"GET_UPTIME"              , request_uptime            },
    {"GET_CPU_TEMP"            , request_core_tempeture_0  },
    {"GET_LOADAGE_1MIN"        , request_loadavg_1min      },
    {"GET_CPU_FREQ_0"          , request_core_freq_0       },
    {"GET_AVAILD_MEM"          , request_avalid_mem        },
    {"GET_ORCHID_ACC"         , request_orchid_ignition   },
    {"GET_HM_ZL"              , request_hourmeter_zl_sec  },
    {"GET_HM_LV"              , request_hourmeter_lv_sec  },
    {"GET_HM"                 , request_hourmeter_sec     },
    {"GET_LAUNCHER_RESTART"   , request_launcher_restart  },
    {"GET_LAUNCHER_HEARTBEAT" , request_launcher_heartbeat},
    {"GET_F1_PRESSED"         , request_kemp_pressed      },
    {"GET_F1_RELEASE"         , request_kemp_release      },
    {"GET_MCU_SW_MAJOR"       , request_mcu_fireware      },
    {"GET_MCU_SW_MINOR"       , request_mcu_fireware_minor},
    {"GET_MCU_HW_MAJOR"       , request_mcu_hardware      },
    {"GET_MCU_HW_MINOR"       , request_mcu_hardware_minor},
    {"GET_MCU_ACC_VOLTAGE"    , request_mcu_acc_voltage   },
    {"GET_MCU_BAT_VOLTAGE"    , request_mcu_bat_voltage   },
    {"AFTER_POWEROFF"         , after_poweroff            },
  };

  customize_actives_queue.reserve(1000);

  int ss = name_activer_map.size();
  if ( ss > 0 ) {
    Active_hook act = name_activer_map[ _op ] ;
    if ( act != nullptr ) {
      printf(" Insert Active: %s Sec=%i Repeat=%i\n",
             _op.c_str(),
             _msec,
             _rep);

      Time_active_t ta;
      ta.active = act;
      ta.sec    = _msec/100;
      customize_actives_queue.push_back(ta);
      printf(" Customize actives:%li\n",
             customize_actives_queue.size());
    } else {
      return -1;
    }
#if _DEBUG
    printf(" Customize actives:%li\n",customize_actives_queue.size());
    info( __FUNCTION__ ,_op.c_str());
#endif
  }
  return 0;
}

// **必须修改,添加到测试流程**
/// "时机-动作"序列
std::vector< Time_active_t > actives_queue {
  { ACTIVE_POINT_MS(200)  ,poweron_battery},
  { ACTIVE_POINT_MS(300)  ,poweron_ignition},
  // 1. system
  { ACTIVE_POINT_MS(21000)  ,request_cpuid},
  { ACTIVE_POINT_MS(21100)  ,request_uptime},
  { ACTIVE_POINT_MS(25200)  ,request_core_tempeture_0},
  { ACTIVE_POINT_MS(25300)  ,request_loadavg_1min},
  { ACTIVE_POINT_MS(25400)  ,request_core_freq_0},
  { ACTIVE_POINT_MS(25500)  ,request_avalid_mem},
  // 2. orchid
  { ACTIVE_POINT_MS(26000) ,request_orchid_ignition},
  // 3. hourmeter
  { ACTIVE_POINT_MS(26100) ,request_hourmeter_zl_sec},
  { ACTIVE_POINT_MS(26200) ,request_hourmeter_lv_sec},
  { ACTIVE_POINT_MS(26300) ,request_hourmeter_sec},

  { ACTIVE_POINT_MS(27100) ,request_hourmeter_zl_sec},
  { ACTIVE_POINT_MS(27200) ,request_hourmeter_lv_sec},
  { ACTIVE_POINT_MS(27300) ,request_hourmeter_sec},
  // 4. launcher
  { ACTIVE_POINT_MS(27400) ,request_launcher_restart},
  { ACTIVE_POINT_MS(27500) ,request_launcher_heartbeat},
  // 5. keypad
  { ACTIVE_POINT_MS(27600) ,request_kemp_pressed},
  { ACTIVE_POINT_MS(27700) ,request_kemp_release},
  // 6. MCU
  { ACTIVE_POINT_MS(29000) ,request_mcu_fireware},
  { ACTIVE_POINT_MS(29100) ,request_mcu_fireware_minor},
  { ACTIVE_POINT_MS(29200) ,request_mcu_hardware},
  { ACTIVE_POINT_MS(29300) ,request_mcu_hardware_minor},
  { ACTIVE_POINT_MS(29400) ,request_mcu_acc_voltage},
  { ACTIVE_POINT_MS(29500) ,request_mcu_bat_voltage},
  //{ ACTIVE_POINT_MS(30000) ,poweroff_ignition}, // 上电,200ms时执行
  //{ ACTIVE_POINT_MS(30100) ,request_orchid_ignition}, // 检查ACC状态
  //{ ACTIVE_POINT_MS(33000) ,poweroff_battery}, // 关机,100ms时执行
                                               
  // 关机后处理,在下次开机之前保留一个间隔时间
  //{ ACTIVE_POINT_MS(35000) ,after_poweroff}, 
};
Test_sys_t  m_sys;

//UDS_op_t  dev_op[ _SIZE_OF_OP ] = {
UDS_op_t  dev_op[] = {
   {"累计启动循环",
        0x0001,0,
        0,0,
        1.0,nullptr},
   {"Testd应答超时",
        0x0002,0,
        0,0,
        1.0,nullptr},
   {"1CF0 标记",
        0x0001,0,
        0,0,
        1.0,nullptr},
   {"1CF1 标记",
        0x0001,0,
        0,0,
        1.0,nullptr},
   {"Testd 标记",
        0x0001,0,
        0,0,
        1.0,nullptr},
   {"Sys.CPU ID",
        0x8000,0,
        0,0,
        1.0,nullptr},
   {"Sys.上电时间(s)",
        0x8001,0,
        0,0,
        1.0,nullptr},
   {"Sys.CPU温度(C)",
        0x8040,0,
        0,0,
        1.0,nullptr},
   {"Sys.平均负载(1min)",
        0x8010,0,
        0,0,
        0.01,nullptr},
   {"Sys.CPU主频(MHz)",
        0x8020,0,
        0,0,
        1.0,nullptr},
   {"Sys.可用内存(MB)",
        0x8031,0,
        0,0,
        1.0,nullptr},
   {"Orchid.钥匙开关",
        0x8201,0,
        0,0,
        1.0,hook_orchid_ignition},
   {"Hyman_x.秒计时(s)",
        0x8101,0,
        0,0,
        1.0,nullptr},
   {"Hyman.秒计时(s)",
        0x8401,0,
        0,0,
        1.0,nullptr},
   {"Hourmeter.秒计时(s)",
        0x8501,0,
        0,0,
        1.0,nullptr},
   {"Launcher.累计重启进程(次)",
        0x8301,0,
        0,0,
        1.0, hook_launcher_restart},
   {"Launcher.心跳计数",
        0x8302,0,
        0,0,
        1.0,nullptr},
   {"Kemp.F1按下计数",
        0x8601,0,
        0,0,
        1.0,nullptr},
   {"Kemp.F1释放计数",
        0x8602,0,
        0,0,
        1.0,nullptr},
   {"Mcu.固件主版本(YQ0.289.)",
        0x8700,0,
        0,0,
        1.0,nullptr},
   {"Mcu.固件次版本",
        0x8701,0,
        0,0,
        1.0,nullptr},
   {"Mcu.硬件主版本",
        0x8702,0,
        0,0,
        1.0,nullptr},
   {"Mcu.硬件次版本",
        0x8703,0,
        0,0,
        1.0,nullptr},
   {"Mcu.ACC电压",
        0x8704,0,
        0,0,
        1.0,hook_mcu_acc_voltage},
   {"Mcu.BAT电压",
        0x8705,0,
        0,0,
        1.0,nullptr},
};

// 回调函数,已注入 canbus 模块中,由接收线程在条件满足时调用
// 应答接受
// 该函数作为回调函数已经注入 canbus模块
// canbus模块已经添加ID过滤,所以进入本函数的
// 消息已确认是UDS应答消息,由回复的ID区分发出设备
// 后,将应答的数据写入对应设备的记录
void receive_ack( struct can_frame* _frame)
{

  // 来自每一个被测试设备的应答,逐一填入该设备对应的UDS_OP中
  int id   = (_frame->can_id & ~0x80000000);
  int addr = (id & 0x000000FF) - 1 ;
  if ( addr < 20 ) {
    Device_t* dev = &m_sys.devs[addr];
    int flag = (id & 0xFFFF0000)>>16;

    if ( flag == 0x0CEF ) {
      printf("UDS Tester <<< 0x%08x %i {",
             _frame->can_id,
             _frame->can_dlc);
      for (int i=0;i<_frame->can_dlc;i++) {
        printf(" %02x",*(_frame->data+i) );
      }
      printf("}\n");
      // 收到 yqsrv_testd 反馈的应答消息,标记进入第二个阶段
      if ((dev->booting_stage & _BOOTING_TESTD) == 0 ) {
        dev->op_list[_OP_SYS_TESTD_BOOTED].result++;
        dev->booting_stage |= _BOOTING_TESTD;
      }

      // UDS 应答
      if ( dev->op_list[_OP_SYS_REQ_TIMEOUE].result> 0 )
        dev->op_list[_OP_SYS_REQ_TIMEOUE].result--;

      int len           = _frame->data[0]-1;
      int response      = (_frame->data[1] - 0x22 );
      dev->responsed    = 0;
      dev->resp_timeout = 0;
      if ( response == 0x40 ) {
        // 正应答,记录有效数据
        for( unsigned int i=0;i<ARRAY_SIZE(dev_op);i++ ) {
          UDS_op_t* op = &dev->op_list[i];
          if ( op->did == m_sys.status.cur_did ){
            memcpy((char*)&op->result,
                   (char*)(_frame->data+2),
                   len);
            if ( op->hook != nullptr ) {
              // 指定参数的后处理钩子
              op->hook( _frame );
            }
            break;
          }
        }
      } else {
        // 负应答
        //printf(" Neg Resposed Did.%04x\n",m_sys.status.cur_did);
      }
    } else if ( flag == 0x1CF0 ) {
      // 自动发送CAN数据,用于标记对方设备已启动
      if ((dev->booting_stage & _BOOTING_1CF0) == 0 ) {
        dev->op_list[_OP_SYS_1CF0].result++;
        dev->booting_stage |= _BOOTING_1CF0;
      }
    } else if ( flag == 0x1CF1 ) {
      //// 自动发送CAN数据,用于标记对方设备已启动
      if ((dev->booting_stage & _BOOTING_1CF1) == 0 ) {
        dev->op_list[_OP_SYS_1CF1].result++;
        dev->booting_stage |= _BOOTING_1CF1;
      }
    } else {
      // 未知数据
    }
  } else {
    // 地址超范围
  }
}

int load_config( std::string& _file )
{
  std::ifstream inFile(_file,std::ios::in);
  if (!inFile.is_open()) {
    return EXIT_FAILURE;
  }

  std::string line;
  while (std::getline(inFile, line)) {
    if ( line.find('#') == std::string::npos ) {
      std::stringstream ss(line);
      std::string item;
      std::vector<std::string> elems;

      while (std::getline(ss, item, ':')) {
        if (!item.empty()) {
          elems.push_back(item);
        }
      }
      if ( elems.size() == 3 ) {
        int    msec    = atoi(elems[0].c_str()) ;
        string op_name = elems[1];
        int    repeat  = atoi(elems[2].c_str()) ;
        if ( insert_op( op_name,msec,repeat) <0) {
        }
      }
    }
  }
  inFile.close();
  return 0 ;
}
// 线程,执行测试流程
void* thread_handler(void* arg)
{
  std::string conf_file("test_config.conf");
  load_config( conf_file );

  (void)arg;
  prctl(PR_SET_NAME,"UdsTester");
  int max_step = customize_actives_queue.size();
  if ( max_step <= 0 ) {
    printf(" [ FATAL ERROR ] Empty actives_queue\n");
    return nullptr;
  }
  int step = 0;
  static int granules_counter = 0;
  static int step_index       = 0;
  static int next_step_sec    = customize_actives_queue[0].sec;

  while ( 1 ) {
    usleep(PROCESS_GRANULES);
    if ( m_sys.status.running == 1 ) {

      if ( m_sys.config.test_enable & _TEST_ENABLE_RANDOM_PWR ) {
        if (m_sys.status.rand_pwr_sec > 0) {
          m_sys.status.rand_pwr_sec--;
        } else {
          poweroff_battery();
          poweroff_ignition();
          m_sys.status.rand_pwr_sec = rand() % m_sys.config.max_rand_pwr_sec;
          step_index    = 0;
          next_step_sec = customize_actives_queue[step_index].sec;
        }
      }

      next_step_sec--;
      if ( next_step_sec <= 0 ) {
        Time_active_t ta = customize_actives_queue[ step_index ];
        if ( ta.active!= nullptr ) {
          ta.active();
        }
        if ( step_index < (max_step-1) ) {
          next_step_sec = customize_actives_queue[step_index+1].sec - ta.sec;
          step_index++;
        } else {
          // 正在执行最后一条
          step_index    = 0;
          next_step_sec = customize_actives_queue[step_index].sec;
        }
      }

      granules_counter++;
      if ( (granules_counter % (1000/GRANULES))==0 ) {
        m_sys.status.preoid_second++;
        m_sys.total_second++;
      }

    }
    // 每个循环周期都发出CAN开关控制命令,确保开关状态符合预期
    set_can_switch ( m_sys.status.can_switch_io);
  }
  return NULL;
}

/////////////////////////////////////////////////
/// 1. System 参数测试
void request_cpuid(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS ) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
    request_DID( 0x8000 ) ;
  }
  return;
}

void request_uptime(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS ) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
    request_DID( 0x8001 ) ;
  }
  return;
}

void request_core_tempeture_0(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS ) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
    request_DID( 0x8040 ) ;
  }
  return;
}

void request_loadavg_1min(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS ) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
    request_DID( 0x8010 ) ;
  }
  return;
}

void request_core_freq_0(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS ) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
    request_DID( 0x8020 ) ;
  }
  return;
}

void request_avalid_mem(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS ) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
    request_DID( 0x8031 ) ;
  }
  return;
}
/////////////////////////////////////////////////
/// 完成 System 参数

/////////////////////////////////////////////////
/// 2. Orchid 参数测试
void request_orchid_ignition(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_ORCHID ) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
    request_DID( 0x8201 ) ;
  }
  return;
}

// 检查内容:检查反馈ACC状态与以下信号的关系:
// 1. 外部控制
// 2. 背光状态
void hook_orchid_ignition( struct can_frame* _f)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_ORCHID ) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
  }
  return ;
}

/////////////////////////////////////////////////
/// 完成 Orchid

/////////////////////////////////////////////////
/// 3. Hourmeter_zl (hourmeter_qt) 参数测试
void request_hourmeter_zl_sec(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_HOURMETER_ZL) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8501 ) ;
  }
  return;
}
void request_hourmeter_lv_sec(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_HOURMETER_LV) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8401 ) ;
  }
  return;
}
void request_hourmeter_sec(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_HOURMETER) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8101 ) ;
  }
  return;
}
void request_launcher_restart(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_LAUNCHER) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8301 ) ;
  }
  return;
}

void hook_launcher_restart( struct can_frame* _f)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_LAUNCHER) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif

  }
  return ;
}

void request_launcher_heartbeat(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_LAUNCHER) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8302 ) ;
  }
  return;
}

///////////////////////////////////////////////////
/// Kemp 按键盘测试
void request_kemp_pressed(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_KEMP) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8601 ) ;
  }
  return;
}
void request_kemp_release(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_KEMP) {
#if _DEBUG
    info( __FUNCTION__ ,nullptr);
#endif
    request_DID( 0x8602 ) ;
  }
  return;
}
///////////////////////////////////////////////////


///////////////////////////////////////////////////
/// MCU 测试
void request_mcu_fireware(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8700 ) ;
  }
  return;
}

void request_mcu_fireware_minor(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8701 ) ;
  }
  return;
}

void request_mcu_hardware(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8702 ) ;
  }
  return;
}

void request_mcu_hardware_minor(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8703 ) ;
  }
  return;
}

void request_mcu_acc_voltage(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8704 ) ;
  }
  return;
}

void hook_mcu_acc_voltage( struct can_frame* _f)
{
  return ;
}

void request_mcu_bat_voltage(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_SYS) {
  #if _DEBUG
    info( __FUNCTION__ ,nullptr);
  #endif
    request_DID( 0x8705 ) ;
  }
  return;
}
///////////////////////////////////////////////////

static void set_can_switch ( const int _io )
{
  struct can_frame tx_frame;
  memset( (char*)tx_frame.data,0x00,8 );
  tx_frame.can_id  = 0x00AA0101 | CAN_EFF_FLAG ;
  tx_frame.can_dlc = 8;
  tx_frame.data[0] = _io;
  Canbus::get_instance()->send_message((char*)&tx_frame,
                                       sizeof(struct can_frame));
  return;
}

// cansend can1 -i 0x00AA0101 -e 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
void poweron_battery(void)
{
#if _DEBUG
  info( __FUNCTION__ ," Turn ON Battery");
#endif
  m_sys.status.bat_status = 1;
  m_sys.status.can_switch_io |= _IGNITION_BITS;
  return ;
}

void poweron_ignition(void)
{
#if _DEBUG
  info( __FUNCTION__ ," Turn ON Ignition");
#endif
  m_sys.status.acc_status     = 1;
  m_sys.status.can_switch_io |= _ACC_BITS;
  return ;
}

void poweroff_ignition(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_RANDOM_PWR ) {
    if ( m_sys.status.rand_pwr_sec > 0 ) {
      return;
    }
  }
#if _DEBUG
  info( __FUNCTION__ ," Turn OFF Ignition");
#endif
  m_sys.status.acc_status     = 0;
  m_sys.status.can_switch_io &= ( ~_ACC_BITS );
  return ;
}

void poweroff_battery(void)
{
  if ( m_sys.config.test_enable & _TEST_ENABLE_RANDOM_PWR ) {
    if ( m_sys.status.rand_pwr_sec > 0 ) {
      return;
    }
  }
#if _DEBUG
  info( __FUNCTION__ ," Turn OFF Battery");
#endif
  m_sys.status.bat_status = 0;
  m_sys.status.can_switch_io &= ( ~_IGNITION_BITS);

  m_sys.status.preoid_cycle++;
  m_sys.total_cycle++;
  // 关机时,检查本次启动过程中检查启动结果并记录
  for ( unsigned int i=0;i<ARRAY_SIZE(m_sys.devs);i++ ){
    Device_t* pDev = &m_sys.devs[i];
    
    if ((pDev->booting_stage & (_BOOTING_1CF0 | 
                                _BOOTING_1CF1 | 
                                _BOOTING_TESTD)) != 0 ) {
      pDev->op_list[_OP_SYS_BOOT_COUNT].result++;
    } else {
      // 三个标志都没有收到,判定为启动失败
      if(m_sys.status.preoid_cycle!=pDev->op_list[_OP_SYS_BOOT_COUNT].result){
        pDev->exception_flag |= _ERR_SYS_BOOT_COUNT;
        pDev->delta_of_boot   = m_sys.status.preoid_cycle -
                                pDev->op_list[_OP_SYS_BOOT_COUNT].result;
      }
    }
  }
  return ;
}

void after_poweroff(void)
{
#if _DEBUG
  info( __FUNCTION__ ," After Poweroff");
#endif
  for ( unsigned int i=0;i<ARRAY_SIZE(m_sys.devs);i++ ){
    Device_t* pDev      = &m_sys.devs[i];
    pDev->booting_stage = 0;
    // 每次关机清除采集的数据(排除启动计数和超时计数)
    //for( unsigned int i=5;i< ARRAY_SIZE(dev_op);i++ ) {
    //  UDS_op_t* op = &pDev->op_list[i];
    //  op->result   = 0;
    //}
  }
  return ;
}


int init_test_sys()
{
  m_sys.total_cycle             = 0;
  m_sys.total_second            = 0;

  m_sys.status.running          = 0;
  m_sys.status.cur_did          = 0;
  m_sys.status.preoid_cycle     = 0;
  m_sys.status.preoid_second    = 0;
  m_sys.status.acc_status       = 0;// 钥匙开关控制状态
  m_sys.status.bat_status       = 0;// 常电控制状态
  m_sys.status.rand_pwr_sec     = 180;
  m_sys.status.can_switch_io    = 0xFF; /// 启动保持全开,方便测试

  m_sys.config.test_enable      = _TEST_ENABLE_SYS; // 默认测试系统参数
  m_sys.config.max_rand_pwr_sec = 180;

  for ( unsigned int i=0;i<ARRAY_SIZE(m_sys.devs);i++ ){
    memset((char*)&m_sys.devs[i],0,sizeof(Device_t));
    memcpy((char*)&m_sys.devs[i].op_list,
           (char*)&dev_op,
           sizeof(UDS_op_t)*ARRAY_SIZE(dev_op));
  }
  return 0;
}

// 一次请求,实际是对所有设备的查询
int request_DID( const int _did ) 
{
  struct can_frame tx_frame;

  m_sys.status.cur_did = _did;
  for ( int i=0;i< ARRAY_SIZE(m_sys.devs);i++ ) {
    Device_t* dev = &m_sys.devs[i];

    // 启动计数差值大于10,停止请求测试

    if ( dev->delta_of_boot < 10 ) {
      dev->op_list[_OP_SYS_REQ_TIMEOUE].result++;

      memset( (char*)tx_frame.data,0xAA,8 );
      tx_frame.can_id  = 0x0CEF0000 | ((i+1)<<8) | CAN_EFF_FLAG ;
      tx_frame.can_dlc = 8;
      tx_frame.data[0] = 0x03;
      tx_frame.data[1] = 0x22;
      tx_frame.data[2] = (_did & 0xFF00)>>8 ;
      tx_frame.data[3] = (_did & 0x00FF) ;

      Canbus::get_instance()->send_message((char*)&tx_frame,
                                           sizeof(struct can_frame));
      printf("UDS Tester >>> 0x%08x DID.%04x \n ",
             tx_frame.can_id, _did);
    }
    //注: 阻塞住主线程,导致UI刷新会延迟
    //usleep(25000);/// 25ms
  }
  return 0;
}

void info( const char* _function,const char* _msg )
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  int ms = tv.tv_usec/1000;
  struct tm * pTM;

  time_t nSeconds;
  time(&nSeconds);
  pTM = localtime(&nSeconds);
  printf("{%02d:%02d:%02d.%03d} [%s]",
          pTM->tm_hour,
          pTM->tm_min,
          pTM->tm_sec,ms,_function);
  if ( _msg != nullptr ) {
    printf(" %s\n",_msg);
  } else {
    printf(" \n");
  }
  return;
}

/// end of code ///
