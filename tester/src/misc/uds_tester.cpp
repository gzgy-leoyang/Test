#include "uds_tester.h"
#include <QDebug>

#include <map>
#include <sys/stat.h>
#include "main.h"
//#include "tester/tester.h"
#include <iterator>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cmath>
#include <thread>

/*
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  #else
  #endif
#endif
*/

using namespace std;

////////////////////////////////////////////////////////////
/// 测试过程由线程按照函数序列执行,Uds_test 模块可以理解为
/// 是 C-CPP 的交互接口

Uds_tester* Uds_tester::m_tester = new Uds_tester();

// 线程,处理测试流程,唯一
pthread_t   Uds_tester::m_pThread;
Test_sys_t  Uds_tester::m_sys;
Active_hook Uds_tester::m_act;

// 当背光改变后,主动重复请求背光传感器状态
//int brightness_changed = 0;

std::vector< Time_active_t >  Uds_tester::actives ;

/// 提供配置文件选择的测试操作
std::map< std::string, Active_hook > Uds_tester::name_activer_map = {
    {"GET_CPU_ID"              , request_cpuid             },
    {"GET_UPTIME"              , request_uptime            },
    {"GET_CPU_TEMP"            , request_core_tempeture_0  },
    {"GET_LOADAGE_1MIN"        , request_loadavg_1min      },
    {"GET_CPU_FREQ_0"          , request_core_freq_0       },
    {"GET_AVAILD_MEM"          , request_avalid_mem        },
    {"GET_CAN0_RX"             , request_can0_rx           },
    {"GET_CAN0_TX"             , request_can0_tx           },
    {"GET_ORCHID_ACC"          , request_orchid_ignition   },
    {"GET_HM_ZL"               , request_hourmeter_zl_sec  },
    {"GET_HM_LV"               , request_hourmeter_lv_sec  },
    {"GET_HM"                  , request_hourmeter_sec     },
    {"GET_LAUNCHER_RESTART"    , request_launcher_restart  },
    {"GET_LAUNCHER_HEARTBEAT"  , request_launcher_heartbeat},
    {"GET_F1_PRESSED"          , request_kemp_pressed      },
    {"GET_F1_RELEASE"          , request_kemp_release      },
    {"GET_TS_PRESS"            , request_kemp_touch_pressed},
    {"GET_TS_RELEASE"          , request_kemp_touch_release},
    {"GET_TS_X"                , request_kemp_touch_x      },
    {"GET_TS_Y"                , request_kemp_touch_y      },
    {"GET_MCU_SW_MAJOR"        , request_mcu_fireware      },
    {"GET_MCU_SW_MINOR"        , request_mcu_fireware_minor},
    {"GET_MCU_HW_MAJOR"        , request_mcu_hardware      },
    {"GET_MCU_HW_MINOR"        , request_mcu_hardware_minor},
    {"GET_MCU_ACC_VOLTAGE"     , request_mcu_acc_voltage   },
    {"GET_MCU_BAT_VOLTAGE"     , request_mcu_bat_voltage   },
    {"GET_BL_SENSOR"           , request_brightnerr_sensor },
    {"GET_BACKLIGHT"           , request_backlight         },
    {"GET_PIEXL_COLOR"         , request_piexl_color       },

    {"GET_RTC_YEAR"            , request_rtc_year          },
    {"GET_RTC_MONTH"           , request_rtc_month         },
    {"GET_RTC_DAY"             , request_rtc_day           },
    {"GET_RTC_HOUR"            , request_rtc_hour          },
    {"GET_RTC_MINUTE"          , request_rtc_min           },
    {"GET_RTC_SECOND"          , request_rtc_sec           },

    {"CHECK_TOUCH_MISMATCH_LT" , check_touch_mismatch_lt   },
    {"CHECK_CAN"               , check_can                 },
    {"CHECK_BACKLIGHT_STATUS"  , check_backlight_status    },
    {"CHECK_ACC_VOLTAGE_GE"    , check_acc_voltage_ge      },
    {"CHECK_BAT_VOLTAGE_GE"    , check_bat_voltage_ge      },
    {"CHECK_ACC_VOLTAGE_LT"    , check_acc_voltage_lt      },
    {"CHECK_BAT_VOLTAGE_LT"    , check_bat_voltage_lt      },
    {"CHECK_COLOR_CHANGE"      , check_color_change        },
    {"CHECK_COLOR_GE"          , check_color_ge            },
    {"CHECK_COLOR_LT"          , check_color_lt            },
    {"CHECK_BOOTING"           , check_booting             },
    {"CHECK_RTC_YEAR"          , check_rtc_year            },
    {"CHECK_RTC_MONTH"         , check_rtc_month           },
    {"CHECK_RTC_DAY"           , check_rtc_day             },

    {"POWER_ON_BAT"            , poweron_battery           },
    {"POWER_ON_ACC"            , poweron_ignition          },
    {"SET_BACKLIGHT"           , set_backlight             },
    {"RECORD_PROFILE"          , profile_record            },
    {"ENABLE_CAN0_TX"          , enable_can0_tx            },
    {"ENABLE_CAN1_TX"          , enable_can1_tx            },
    {"DISABLE_CAN0_TX"         , disable_can0_tx           },
    {"DISABLE_CAN1_TX"         , disable_can1_tx           },
    {"RANDOM_POWEROFF"         , random_poweroff           },
    {"POWER_OFF_ACC"           , poweroff_ignition         },
    {"POWER_OFF_BAT"           , poweroff_battery          },
    {"AFTER_POWEROFF"          , after_poweroff            },
    {"SET_ACC_BIT"             , set_acc_bit               },
    {"SET_BAT_BIT"             , set_bat_bit               },
    {"SET_TOUCH_COUNTER"       , set_kemp_touch_counter    },
    {"SET_DEVICE_NUM"          , set_device_number         },
    {"SET_PIXEL_X"             , set_lcd_pixel_x           },
    {"SET_PIXEL_Y"             , set_lcd_pixel_y           },
    {"ENABLE_QUIT_EXCEPTION"   , enable_quit_exception     },
    {"MAX_COLOR_CHANGED"       , set_max_color_changed_cnt },
};

/// 测试参数模板,影响列表的显示内容,保存读取的结果
UDS_op_t  Uds_tester::dev_op[] = {
  // 测试过程的记录数据
   {"累计启动循环(次)",
        0x8001,0,
        0,0,
        1.0,nullptr},
   {"Testd应答超时(次)",
        0x8002,0,
        0,0,
        1.0,nullptr},
   {"设备启动标记1CF0(次)",
        0x8003,0,
        0,0,
        1.0,nullptr},
   {"设备启动标记1CF1(次)",
        0x8004,0,
        0,0,
        1.0,nullptr},
   {"yqsrv_testd 启动标记(次)",
        0x8005,0,
        0,0,
        1.0,nullptr},
   {"UDS否定应答(次)",
        0x8006,0,
        0,0,
        1.0,nullptr},
   {"检查中止(次)",
        0x8007,0,
        0,0,
        1.0,nullptr},
   {"UI 启动耗时(s)",
        0x8008,0,
        0,0,
        1.0,nullptr},
   {"UI 启动最大耗时(s)",
        0x8009,0,
        0,0,
        1.0,nullptr},

// 被测试设备的系统数据
   {"Sys.CPU ID",
        0x8100,0,
        0,0,
        1.0,nullptr},
   {"Sys.上电时间(s)",
        0x8101,0,
        0,0,
        1.0,nullptr},
   {"Sys.CPU温度(C)",
        0x8140,0,
        0,0,
        1.0,nullptr},
   {"Sys.平均负载(1min)",
        0x8110,0,
        0,0,
        0.01,nullptr},
   {"Sys.CPU主频(MHz)",
        0x8120,0,
        0,0,
        1.0,nullptr},
   {"Sys.可用内存(MB)",
        0x8131,0,
        0,0,
        1.0,nullptr},
   {"Sys.CAN0接收帧",
        0x8150,0,
        0,0,
        1.0,nullptr},
   {"Sys.CAN0发送帧",
        0x8151,0,
        0,0,
        1.0,nullptr},
   {"Sys.亮度",
        0x8142,0,
        0,0,
        1.0,nullptr},
   {"Sys.亮度传感器",
        0x8143,0,
        0,0,
        1.0,nullptr},
// 被测试设备服务数据
   {"Orchid.钥匙开关",
        0x8201,0,
        0,0,
        1.0,nullptr},

   {"Launcher.累计重启进程(次)",
        0x8301,0,
        0,0,
        1.0, nullptr},
   {"Launcher.心跳计数",
        0x8302,0,
        0,0,
        1.0,nullptr},

   {"Hyman_x.秒计时(s)",
        0x8400,0,
        0,0,
        1.0,nullptr},
   {"Hyman.秒计时(s)",
        0x8401,0,
        0,0,
        1.0,nullptr},
   {"Hourmeter.秒计时(s)",
        0x8402,0,
        0,0,
        1.0,nullptr},

   {"Input.F1按下计数(次)",
        0x8601,0,
        0,0,
        1.0,nullptr},
   {"Input.F1释放计数(次)",
        0x8602,0,
        0,0,
        1.0,nullptr},
   {"Input.触摸按下计数(次)",
        0x8603,0,
        0,0,
        1.0,nullptr},
   {"Input.触摸释放计数(次)",
        0x8604,0,
        0,0,
        1.0,nullptr},
   {"Input.触摸坐标X(Piexl)",
        0x8605,0,
        0,0,
        1.0,nullptr},
   {"Input.触摸坐标Y(Piexl)",
        0x8606,0,
        0,0,
        1.0,nullptr},
// 被测试设备的MCU数据
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
   {"Mcu.ACC电压(mV)",
        0x8704,0,
        0,0,
        1.0,nullptr},
   {"Mcu.BAT电压(mV)",
        0x8705,0,
        0,0,
        1.0,nullptr},

   {"LCD.指定像素颜色(0x00RRGGBB)",
        0x8800,0,
        0,0,
        1.0,nullptr},

   {"RTC.年",
        0x8900,0,
        0,0,
        1.0,nullptr},
   {"RTC.月",
        0x8901,0,
        0,0,
        1.0,nullptr},
   {"RTC.日",
        0x8902,0,
        0,0,
        1.0,nullptr},
   {"RTC.时",
        0x8903,0,
        0,0,
        1.0,nullptr},
   {"RTC.分",
        0x8904,0,
        0,0,
        1.0,nullptr},
   {"RTC.秒",
        0x8905,0,
        0,0,
        1.0,nullptr},
};

typedef struct {
  int total_cycle ;
  int total_second;
} Test_record_t;

Uds_tester::
Uds_tester( QObject* parent ):
  QObject(parent)
{
  LOGV << "Test Starting...";
  Uds_tester::event_record(QString("***** Testing start *****"));
  // 准备测试过程数据,包括全部被测设备的数据
  init_test_sys();
  // 加载测试统计数据
  Test_record_t rec ;
  memset( (char*)&rec,0,sizeof(Test_record_t));
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  LOGV << "Loading Test Statistics...";
  if ( read_data("/home/root/Tester/test.stats",
            (char*)&rec,
            sizeof( Test_record_t )) < 0 ) {
    // 全新文件,按32个设备计
    write_data("/home/root/Tester/test.stats",
              (char*)&rec,
              sizeof( Test_record_t ));
  }
  #else
  #endif
#endif
  m_sys.total_cycle       = rec.total_cycle;
  m_sys.total_second      = rec.total_second;
  // 默认CAN较高测试压力,自动运行
  Canbus::get_instance()->set_tx_load(0,1);
  Canbus::get_instance()->set_tx_load(1,1);
  m_sys.status.running = 1;
  // 向CAN模块注册接收回调函数,用于接收CAN应答
  Canbus::get_instance()->set_receive_callback(Uds_tester::receive_ack);
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  // 启动测试控制线程
  LOGV << "Creating test thread...";
  if ( pthread_create(&Uds_tester::m_pThread,
                      NULL,
                      thread_handler,
                      NULL) < 0 ){
    LOGW << "Failed to create test thread";
    return ;
  }
  #else
  #endif
#endif
}

//////////////////////////////////////////////////////////////////////
// 回调函数,已注入 canbus 模块中,由接收线程在条件满足时调用
// 应答接受
// 该函数作为回调函数已经注入 canbus模块
// canbus模块已经添加ID过滤,所以进入本函数的
// 消息已确认是UDS应答消息,由回复的ID区分发出设备
// 后,将应答的数据写入对应设备的记录
void Uds_tester::receive_ack( struct can_frame* _frame)
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  #else
  #endif
#endif
  // 来自每一个被测试设备的应答,逐一填入该设备对应的UDS_OP中
  int id   = (_frame->can_id & ~0x80000000);
  if ( id == 0x00AA0301 ) {
    long int sensor = 0; 
    memcpy( &sensor,
            _frame->data,
            8 );
    LOGV<<"Receive Brightness Sensor:"<<sensor;
    for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
      int br = static_cast<int>( (sensor>>(i*2)) & 0x0000000000000003);
      Device_t* dev       = &m_sys.devs[i];
      Testing_param_t* pp = get_param_by_did( 0x8143, dev) ;
      if ( pp != nullptr )  {
        pp->val = br;
      }
    }
  } else {
    int addr = (id & 0x000000FF) - 1 ;
    if (addr <= 16 ) {
      Device_t* dev = &m_sys.devs[addr];
      // 地址指向设备
      int flag = (id & 0xFFFF0000)>>16;
      // 设备内部,根据 ID头 的标记,区别不同处理
      if ( flag == 0x0CEF ) {
        LOGV<<"[Receive responed]"<<std::hex<<_frame->can_id
            <<"{"<<_frame->data[0]
            <<" ,"<<_frame->data[1]
            <<" ,"<<_frame->data[2]
            <<" ,"<<_frame->data[3]
            <<" ,"<<_frame->data[4]
            <<" ,"<<_frame->data[5]
            <<" ,"<<_frame->data[6]
            <<" ,"<<_frame->data[7]<<"}";

        // 收到 yqsrv_testd 反馈的应答消息,标记进入第二个阶段
        if ((dev->booting_stage & _BOOTING_TESTD) == 0 ) {
          dev->booting_stage |= _BOOTING_TESTD;
          Testing_param_t* p = get_param_by_did(0x8005,dev) ;
          if ( p != nullptr ) {
            p->val++;
          }
        }

        Testing_param_t* p = get_param_by_did(0x8002,dev) ;
        if ( p != nullptr ) {
          if ( p->val > 0 ) {
            p->val--;
          }
        }

        //int response = 0 ;
        int did      = 0 ;
        int sid      = 0 ;
        int len      = _frame->data[0]-3; //减去[1:3]字节
        if ( _frame->data[1] == 0x7F ) {
          sid = _frame->data[2];
          dev->negitive_responed_cnt++;
          LOGW<<" Negitive responed:"<<sid;
          return ;
        } else {
          // 正应答,记录有效数据
          sid = (_frame->data[1] - 0x40 );
          did = _frame->data[2];
          did = (did<<8) | _frame->data[3];

          dev->responsed    = 0;
          dev->resp_timeout = 0;
          int temp = 0;
          switch ( len ) {
            case 1:
              temp = *(_frame->data+4);
              break;
            case 2:
              temp = *(_frame->data+4);
              temp =  (temp <<8) | *(_frame->data+5);
              break;
            case 4:
              temp = *(_frame->data+4);
              temp = (temp <<8) | *(_frame->data+5);
              temp = (temp <<8) | *(_frame->data+6);
              temp = (temp <<8) | *(_frame->data+7);
              break;
          }

          if  ( sid == 0x22 ) {
            // 直接将数据按SID,DID写入矩阵,后续显示内容按照SID/DID取数据显示
            int unit_index  = ((did-0x8000) >> 8) & 0x1F;
            int param_index = (did & 0x00FF);
            if ( (unit_index < 32) && (param_index < 255) ){
              Testing_param_t* pParam = &dev->val_matrix[unit_index][param_index]; 
              pParam->val = temp;
            }
          }
        }
      } else if ( flag == 0x1CF0 ) {
        // 自动发送CAN数据,用于标记对方设备已启动
        if ((dev->booting_stage & _BOOTING_1CF0) == 0 ) {
          dev->booting_stage |= _BOOTING_1CF0;
          Testing_param_t* p = get_param_by_did(0x8003,dev) ;
          if ( p!= nullptr) {
            p->val++;
          }
        }
      } else if ( flag == 0x1CF1 ) {
        //// 自动发送CAN数据,用于标记对方设备已启动
        if ((dev->booting_stage & _BOOTING_1CF1) == 0 ) {
          dev->booting_stage |= _BOOTING_1CF1;
          Testing_param_t* p = get_param_by_did(0x8004,dev) ;
          if ( p!= nullptr) {
            p->val++;
          }
        }
      } else if ( flag == 0x1CF2 ) {
        // 自动发送CAN数据,用于标记对方设备已启动
        Testing_param_t* p = get_param_by_did(0x8008,dev) ;
        int sec = *(_frame->data+0);
        if ( p!= nullptr) {
          if ( p->val == 0xFFFFFFFF ) {
            p->val = sec;
            Testing_param_t* pp = get_param_by_did(0x8009,dev) ;
            if ( pp != nullptr ) {
              if ( pp->val < p->val ) {
                pp->val = p->val;
              }
            }
          }
        }
      } else {
        // 未知数据
      }
    } else {
      // 地址超范围
    }
  }
}

//////////////////////////////////////////////////////////////////////
// 线程,执行测试流程
int uds_tester_step_index = 0;

void* Uds_tester::thread_handler(void* arg)
{
  LOGI << "Test thread"<<std::endl;
  m_sys.config_file_name = std::string("等待加载...");
  (void)arg;
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  // Uds_Tester为全局静态的单例对象,主进程一旦启动,线程就会启动
  // 如果在此直接加载配置,有可能由于main中的 config_file_name 还没有填充
  // 导致配置失败而退出,所以需要等一会儿,等Qt的命令行解析工具填充参数
  sleep(10);

  prctl(PR_SET_NAME,"UdsTester");

  LOGV << " Loading test process..."<<std::endl;
  std::string conf_file("default_test.conf");
  if ( config_file_name[0] != '\0' ) {
    conf_file = std::string( config_file_name);
  }
  Uds_tester::load_config( conf_file );
  int max_step = actives.size();
  if ( max_step <= 0 ) {
    LOGE<<"[ FATAL ERROR ] Empty customize_actives_queue";
    return nullptr;
  }
  LOGV << " Load test process...OK"<<std::endl;
  m_sys.config_file_name = conf_file;

  static int granules_counter = 0;
  static int step_index       = 0;
  static int next_step_sec    = actives[0].sec;
  m_sys.status.total_step = max_step;

  static int ff03_ff01_flag   = 1;
  uds_tester_step_index = 0;
  while ( 1 ) {
    usleep(PROCESS_GRANULES);
    if ( m_sys.status.running == 1 ) {
      m_sys.status.tick_counter++;

      if ( m_sys.status.random_poweroff_sec > 0 ) {
        // 如果设置了随机关机时间,则执行以下
        m_sys.status.random_poweroff_sec--;
        if ( m_sys.status.random_poweroff_sec == 0 ) {
          // 随机时间到,关机
          LOGV << "Random time over,shutdowning..."<<std::endl;
          // 正在执行最后一条
          step_index                      = 0;
          next_step_sec                   = actives[step_index].sec;
          m_sys.status.process_step_index = step_index;
          uds_tester_step_index           = step_index;
          // 随机关机时,也需要记录统计数据
          Uds_tester::stats_record();

          // 随机时间到,关ACC后,延时关闭BAT
          poweroff_ignition(0);
          sleep(5);
          LOGV << "Turn off battery..."<<std::endl;
          poweroff_battery(0);
          // 下一个循环周期,由脚本定义动作生产新的随机值,
          // 肯定是小于以下数值的
          m_sys.status.random_poweroff_sec = 0xffffffff;
          m_sys.status.tick_counter = 0;
        }
      }

      next_step_sec--;
      //m_sys.status.next_step_sec = next_step_sec;
      if ( next_step_sec <= 0 ) {
        // 测试动作倒计时到,带参数执行动作函数
        Time_active_t ta = actives[ step_index ];
        m_act = ta.active;
        if ( m_act!= nullptr ) {
          (*m_act)(ta.param);
        }
        // 更新倒计时,等待下一步测试动作
        if ( step_index < (max_step-1) ) {
          next_step_sec = actives[step_index+1].sec - ta.sec;
          step_index++;
        } else {
          // 正在执行最后一条
          // 如果随机时间还没有到,则复位该时间,下次重新计算
          m_sys.status.random_poweroff_sec = 0xffffffff;
          step_index    = 0;
          next_step_sec = actives[step_index].sec;
          // 每个循环周期结束时记录统计
          Uds_tester::stats_record();
          m_sys.status.tick_counter = 0;
        }
        m_sys.status.process_step_index = step_index;
        m_sys.status.next_step_sec      = next_step_sec;
        //LOGV<<"Test Executing..."
        //<<" Step."<<m_sys.status.process_step_index
        //<<" Next."<<m_sys.status.next_step_sec
        //<<" Tick."<<m_sys.status.tick_counter<<std::endl;
        uds_tester_step_index = step_index;
      }

      granules_counter++;
      if ( (granules_counter % (1000/GRANULES))==0 ) {
        m_sys.status.preoid_second++;
        m_sys.total_second++;
      }

    }
    // 即使目前没有运行测试,还是保持控制命令
    if ( ff03_ff01_flag == 1 ) {
      ff03_ff01_flag = 3;
      // 发出IO控制命令,或是查询背光传感器状态
      request_brightnerr_sensor(0);
    } else {
      ff03_ff01_flag = 1;
      // 每个循环周期都发出CAN开关控制命令,确保开关状态符合预期
      set_can_switch ( m_sys.status.can_switch_io);
    }
  }
  #else
  #endif
#endif
  return NULL;
}

//////////////////////////////////////////////////////////////////////
// 测试器内部操作
int Uds_tester::
init_test_sys()
{
  // 准备记录数据文件路径
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  char dir_path[128] ={"/home/root/Tester/"};
  if(access(dir_path, NULL)!=0) {
    if(mkdir(dir_path, S_IWRITE|S_IREAD)<0) {
        return -1;
    }
  }

  char dev_dir_path[128];
  char dev_name[32];
  for ( int i=0;i<32;i++ ) {
    memset( dev_dir_path,'\0',128 );
    memset( dev_name,'\0',32);

    strcpy( dev_dir_path,dir_path );
    sprintf( dev_name,"dev_%i/",i );
    strcat( dev_dir_path,dev_name );

    if(access(dev_dir_path, NULL)!=0) {
      if(mkdir(dev_dir_path, S_IWRITE|S_IREAD)<0) {
          return -1;
      }
      LOGV<<"Creating: "<<dev_dir_path<<std::endl;
    }
  }
  #else
  #endif
#endif

  
  m_sys.total_cycle             = 0;
  m_sys.total_second            = 0;
  m_sys.sizeof_dev_tested       = 0;

  m_sys.status.running          = 0;
  m_sys.status.cur_did          = 0;
  m_sys.status.preoid_cycle     = 0;
  m_sys.status.preoid_second    = 0;
  m_sys.status.acc_status       = 0;// 钥匙开关控制状态
  m_sys.status.bat_status       = 0;// 常电控制状态
  m_sys.status.rand_pwr_sec     = 180;
  m_sys.status.can_switch_io    = 0xFF; /// 启动保持全开,方便测试
  m_sys.status.process_step_index= 0; 
  m_sys.status.total_step       = 0; 
  m_sys.status.next_step_sec    = 0; 
  m_sys.status.acc_bit          = 0;
  m_sys.status.bat_bit          = 0;
  m_sys.status.max_color_unchanged_cnt = 1;

  m_sys.config.max_rand_pwr_sec = 180;

  // 启动时按16个设备准备数据,实际运行过程根据脚本配置执行
  LOGV<<" Initilizing device matrix..."<<std::endl;
  for ( unsigned int i=0;i<16;i++ ){
    // 初始化设备数据
    // 测试返回值矩阵,不再保持公共的,固定的属性字段
    memset( (char*)&m_sys.devs[i],0,sizeof(Device_t));
    //memset( (char*)&m_sys.devs[i].val_matrix,
    //        0xff,
    //        sizeof(Testing_param_t)*32*256 );
  }
  return 0;
}

int Uds_tester::
get_val_except_by_did( int _did,
                  Device_t* _dev,
                  int * _val,
                  int* _except)
{
  (void)_did;
  (void)_dev;
  (void)_val;
  (void)_except;

  if ( _dev == nullptr )
    return -1;

  int unit_index  = ((_did-0x8000) >> 8) & 0x1F;
  int param_index = (_did & 0x00FF);
  *_val = _dev->val_matrix[unit_index][param_index].val; 
  *_except = _dev->val_matrix[unit_index][param_index].exception_flag; 
  return 0;
}

int Uds_tester::
set_value_by_did( int _did,
                  Device_t* _dev,
                  int _val )
{
  (void)_did;
  (void)_val;

  if ( _dev == nullptr )
    return -1;

  int unit_index  = ((_did-0x8000) >> 8) & 0x1F;
  int param_index = (_did & 0x00FF);
  _dev->val_matrix[unit_index][param_index].val = _val; 
  return 0;
}

Testing_param_t* Uds_tester::
get_param_by_did( int _did,Device_t* _dev)
{
  if ( _dev == nullptr )
    return nullptr;

  int unit_index  = ((_did-0x8000) >> 8) & 0x1F;
  int param_index = (_did & 0x00FF);
  if ( (unit_index>=32) ||(param_index>=256) ){
    return nullptr;
  }
  return &_dev->val_matrix[unit_index][param_index]; 
}

int Uds_tester::
set_DID(const int _did,const int _val) 
{
  struct can_frame tx_frame;

  m_sys.status.cur_did = _did;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* dev = &m_sys.devs[i];
    // 启动计数差值大于10,停止请求测试
    if ( dev->delta_of_boot < 10 ) {
      //dev->op_list[_OP_SYS_REQ_TIMEOUE].result++;

      memset( (char*)tx_frame.data,0xAA,8 );
      tx_frame.can_id  = 0x0CEF0000 | ((i+1)<<8) | CAN_EFF_FLAG ;
      tx_frame.can_dlc = 8;
      tx_frame.data[0] = 0x07;
      tx_frame.data[1] = 0x2E;
      tx_frame.data[2] = (_did & 0xFF00)>>8 ;
      tx_frame.data[3] = (_did & 0x00FF) ;

      tx_frame.data[4] = (_val & 0xFF000000)>>24;
      tx_frame.data[5] = (_val & 0x00FF0000)>>16;
      tx_frame.data[6] = (_val & 0x0000FF00)>>8;
      tx_frame.data[7] = (_val & 0x000000FF);

      Canbus::get_instance()->send_message((char*)&tx_frame,
                                           sizeof(struct can_frame));
      //#if _DEBUG
      //printf("UDS Tester >>> 0x%08x DID.%04x \n ",
      //       tx_frame.can_id, _did);
      //#endif
    }
  }
  return 0;
}
// 一次请求,实际是对所有设备的查询
void Uds_tester::
request_brightnerr_sensor(const int _param) 
{
  (void)_param;
  struct can_frame tx_frame;

  memset( (char*)tx_frame.data,0x00,8 );
  tx_frame.can_id  = 0x00AA0301 | CAN_EFF_FLAG ;
  tx_frame.can_dlc = 8;
  Canbus::get_instance()->send_message((char*)&tx_frame,
                                       sizeof(struct can_frame));
  LOGV<<"Rquesting the status of brihgtness sensor..."<<std::endl;
  return ;
}
// 一次请求,实际是对所有设备的查询
int Uds_tester::
request_DID( const int _did ) 
{
  struct can_frame tx_frame;

  m_sys.status.cur_did = _did;
  LOGV<<"Rquesting DID: 0x"<<std::hex<<_did<<std::endl;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* dev = &m_sys.devs[i];

    // 在请求数据之前,将对应数据区清除,标记为无效数据0xFFFFFFFF;
    // 防止在检查数据过程中,由于对侧设备的负应答,采用默认的0数据导致误判
    Testing_param_t* p = get_param_by_did( _did, dev) ;
    if ( p != nullptr ) {
      p->val = 0xffffffff;
    } 
    // 启动计数差值大于10,停止请求测试
    if ( dev->delta_of_boot < 10 ) {
      //dev->op_list[_OP_SYS_REQ_TIMEOUE].result++;

      memset( (char*)tx_frame.data,0xAA,8 );
      tx_frame.can_id  = 0x0CEF0000 | ((i+1)<<8) | CAN_EFF_FLAG ;
      tx_frame.can_dlc = 8;
      tx_frame.data[0] = 0x03;
      tx_frame.data[1] = 0x22;
      tx_frame.data[2] = (_did & 0xFF00)>>8 ;
      tx_frame.data[3] = (_did & 0x00FF) ;
      Canbus::get_instance()->send_message((char*)&tx_frame,
                                           sizeof(struct can_frame));
      LOGV<<"[Transmit request]"<<std::hex<<tx_frame.can_id
          <<"{"<<tx_frame.data[0]
          <<" ,"<<tx_frame.data[1]
          <<" ,"<<tx_frame.data[2]
          <<" ,"<<tx_frame.data[3]
          <<" ,"<<tx_frame.data[4]
          <<" ,"<<tx_frame.data[5]
          <<" ,"<<tx_frame.data[6]
          <<" ,"<<tx_frame.data[7]<<"}";
    }
  }
  return 0;
}

void Uds_tester::
set_can_switch ( const int _io )
{
  struct can_frame tx_frame;
  memset( (char*)tx_frame.data,0x00,8 );
  tx_frame.can_id  = 0x00AA0101 | CAN_EFF_FLAG ;
  tx_frame.can_dlc = 8;
  tx_frame.data[0] = _io;
  Canbus::get_instance()->send_message((char*)&tx_frame,
                                       sizeof(struct can_frame));

  LOGV<<"Set CAN IO:0x"<<std::hex<<_io<<std::endl;
  return;
}

int Uds_tester::
read_data( const char* _file_name, 
           char* _buf,
           const int _size )
{
  if (access( _file_name,F_OK | R_OK ) == 0 ) {
    int fd = open ( _file_name,O_RDONLY );
    if ( fd < 0 ) {
      return -1;
    } else {
      memset( _buf,0,_size );

      int re_try = 0;
      do {
        ssize_t ret = read(fd,_buf,_size);
        if ( ret < _size ) {
          re_try++;
          usleep(1000*re_try);
        } else {
          re_try = 0;
        }
      } while ((re_try>0) && (re_try<5));
      close(fd);
    }
  }
  return 0;
}

int Uds_tester::
write_data( const char* _file_name ,
            char *_buf,
            const int _size )
{
  int fd = open( _file_name,
                 O_RDWR|O_CREAT|O_TRUNC,
                 S_IRUSR|S_IWUSR);
  if ( fd < 0 ) {
    return -1;
  }

  int re_write = 0;
  do {
    lseek(fd,0,SEEK_SET);
    ssize_t ret = write(fd, _buf,_size);
    if ( ret < _size ) {
      re_write++;
      usleep(1000*re_write);
    }
  } while ((re_write>0)&&(re_write<5));

  if ( fdatasync(fd) < 0 ) {
    usleep(5000);
    sync();
  }
  close(fd);
  return 0;
}

int Uds_tester::
insert_op( std::string& _op,int _msec,int _rep )
{

  actives.reserve(1000);

  int ss = name_activer_map.size();
  if ( ss > 0 ) {
    Active_hook act = name_activer_map[ _op ] ;
    if ( act != nullptr ) {
      Time_active_t ta;
      ta.active = act;
      ta.sec    = _msec/100;
      ta.param  = _rep;
      actives.push_back(ta);
      LOGI<<"+ Testing Active["
        <<actives.size()
        <<"] "
        <<_op.c_str()
        <<" Sec."
        <<_msec
        <<" Param."
        <<_rep;
    } else {
      return -1;
    }
  }
  return 0;
}

// 加载测试流程控制脚本
int Uds_tester::load_config( std::string& _file )
{
  std::ifstream inFile(_file,std::ios::in);
  if (!inFile.is_open()) {
    LOGW<<" Could NOT found config file";
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

        m_sys.status.tick_max = msec/1000;
      }
    }
  }
  inFile.close();
  return 0 ;
}


// gst-launch-1.0 imxv4l2videosrc device=/dev/video1 num-buffers=1 ! jpegenc ! filesink location=eeee.jpg
void Uds_tester::
capture_image(const char* _tips)
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  struct timeval tv;
  gettimeofday(&tv,NULL);
  //int ms = tv.tv_usec/1000;
  time_t nSeconds;
  struct tm * pTM;
  time(&nSeconds);
  pTM = localtime(&nSeconds);

  std::stringstream ss;
  ss<<"gst-launch-1.0 imxv4l2videosrc device=/dev/video1 num-buffers=1 ! jpegenc ! filesink location="
    <<_tips<<"#"
    <<(pTM->tm_year+1900)<<"-"
    <<(pTM->tm_mon+1)<<"-"
    <<pTM->tm_mday<<"_"
    <<pTM->tm_hour<<":"
    <<pTM->tm_min<<":"
    <<pTM->tm_sec<<".jpg";
  std::string cmd_str = ss.str();
  std::system(cmd_str.data());
  LOGW<<" Capture image";
  #else
  #endif
#endif
  return ;
}

// 记录测试过程的统计信息,比如:累计运行秒数,累计运行循环计数等
int Uds_tester::
stats_record(void)
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  Test_record_t rec ;
  rec.total_cycle  = m_sys.total_cycle ;
  rec.total_second = m_sys.total_second ;

  int fd = open( "/home/root/Tester/test.stats",
                 O_RDWR|O_CREAT|O_TRUNC,
                 S_IRUSR|S_IWUSR);
  if ( fd < 0 ) {
    return -1;
  }

  int re_write = 0;
  do {
    lseek(fd,0,SEEK_SET);
    ssize_t ret = write(fd, &rec,sizeof( Test_record_t ));
    if (ret < sizeof( Test_record_t )) {
      re_write++;
      usleep(1000*re_write);
    }
  } while ((re_write>0)&&(re_write<5));

  if ( fdatasync(fd) < 0 ) {
    usleep(5000);
    sync();
  }
  close(fd);
  #else
  #endif
#endif

  return 0 ;
}
void Uds_tester::
event_record(const QString& _str)
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  struct timeval tv;
  gettimeofday(&tv,NULL);
  int ms = tv.tv_usec/1000;
  time_t nSeconds;
  struct tm * pTM;
  time(&nSeconds);
  pTM = localtime(&nSeconds);

  QString log_file_name = QString("/home/root/Tester/test.event");
  QFile f( log_file_name );
  if (f.open( QIODevice::WriteOnly|
              QIODevice::Text |
              QIODevice::Append )){
    QString str = QString("{%1-%2-%3 %4:%5:%6.%7} "+_str)
                .arg(pTM->tm_year-100)
                .arg(pTM->tm_mon+1)
                .arg(pTM->tm_mday)
                .arg(pTM->tm_hour)
                .arg(pTM->tm_min)
                .arg(pTM->tm_sec)
                .arg(ms);
    QString info1 = QString(" <Current> Tick.%1(0.1s) Step.%2 ACC.%3 BAT.%4")
                   .arg(m_sys.status.tick_counter)
                   .arg(m_sys.status.process_step_index)
                   .arg(m_sys.status.acc_status)
                   .arg(m_sys.status.bat_status);
    QString info2 = QString(" <Statistics> Peroid cycle:%1 Sec:%2 Total cycle:%3 Sec.%4")
                   .arg(m_sys.status.preoid_cycle)
                   .arg(m_sys.status.preoid_second)
                   .arg(m_sys.total_cycle )
                   .arg(m_sys.total_second);
    QTextStream out(&f);
    out<<str<<"\n";
    out<<info1<< "\n";
    out<<info2<< "\n";
    out<<"----------------------------"<< "\n";
    f.close();
    sync();
  } else {
    LOGW<<" Event record failed";
  }
  #else
  #endif
#endif
}


string head_row( void )
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  string line_str;
  line_str.append( "消息级别, ");
  line_str.append( "日期, ");
  line_str.append( "时间, ");

  line_str.append( "测试循环计数, ");
  line_str.append( "累计循环计数, ");
  line_str.append( "测试时长, ");
  line_str.append( "累计测试时长, ");
  line_str.append( "CAN0总线负载, ");
  line_str.append( "CAN1总线负载, ");
  line_str.append( "ACC状态, ");
  line_str.append( "BAT状态, ");
  // 添加各测试项目名称
  for ( int j=0;j<_SIZE_OF_OP;j++ ) {
    char buf[128];
    std::sprintf(buf,"%s, ",(Uds_tester::dev_op[j].desp));
    string val_str = string(buf);
    line_str.append( val_str );
  }
  return line_str;
  #else
    return std::string("string");
  #endif
#endif
}

string parameter_row( Device_t* _pDev, const int _type )
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  #else
  #endif
#endif
  string line_str;
  string val_str;

  if ( _type == 0 ) {
    line_str.append( "Info, ");
  } else if (_type == 1) {
    line_str.append( "Warning, ");
  } else if (_type == 2) {
    line_str.append( "Error, ");
  }

  auto now= chrono::system_clock::now();
  time_t time   = chrono::system_clock::to_time_t( now);
  struct tm* tt = std::localtime( &time );
  char str_time[32];
  memset( str_time,'\0',32 );
  if (std::strftime(str_time,
                    32,
                    "%Y-%m-%d, ",
                    tt)){
    line_str.append( str_time);
  }

  if (std::strftime(str_time,
                    32,
                    "%H:%M:%S, ",
                    tt)){
    line_str.append( str_time);
  }

  char buf[32];
  std::sprintf(buf,"%i, ",Uds_tester::m_sys.status.preoid_cycle);
  val_str = string(buf);
  line_str.append( val_str );
  memset( buf,'\0',32 );

  std::sprintf(buf,"%i, ",Uds_tester::m_sys.total_cycle);
  val_str = string(buf);
  line_str.append( val_str );
  memset( buf,'\0',32 );

  int hour = Uds_tester::m_sys.status.preoid_second/ 3600;
  int temp = Uds_tester::m_sys.status.preoid_second% 3600;
  int min  = temp / 60;
  int sec  = temp % 60;
  std::sprintf(buf,"%i:%i:%i, ",hour,min,sec);
  val_str = string(buf);
  line_str.append( val_str );
  memset( buf,'\0',32 );

  hour = Uds_tester::m_sys.total_second/ 3600;
  temp = Uds_tester::m_sys.total_second% 3600;
  min  = temp / 60;
  sec  = temp % 60;
  std::sprintf(buf,"%i:%i:%i, ",hour,min,sec);
  val_str = string(buf);
  line_str.append( val_str );
  memset( buf,'\0',32 );

  std::sprintf(buf,"%.02f, ",Uds_tester::m_sys.status.can_load_0);
  val_str = string(buf);
  line_str.append( val_str );
  memset( buf,'\0',32 );

  std::sprintf(buf,"%.02f, ",Uds_tester::m_sys.status.can_load_1);
  val_str = string(buf);
  line_str.append( val_str );
  memset( buf,'\0',32 );

  std::sprintf(buf,"%i, ",Uds_tester::m_sys.status.acc_status);
  val_str = string(buf);
  line_str.append( val_str );
  memset( buf,'\0',32 );

  std::sprintf(buf,"%i, ",Uds_tester::m_sys.status.bat_status);
  val_str = string(buf);
  line_str.append( val_str );
  memset( buf,'\0',32 );

  for ( int j=0;j<_SIZE_OF_OP;j++ ) {
    UDS_op_t* op = & Uds_tester::dev_op[j];

    Testing_param_t* p = nullptr;
    p = Uds_tester::get_param_by_did(op->did,_pDev);
    if ( p != nullptr) {
      if ( 1 == static_cast<int>(op->rate) ) {
        val_str = to_string(p->val);
      } else {
        char buf[5]={0,0,0,0,0};
        std::sprintf(buf,"%.02f",(p->val*op->rate));
        val_str = string(buf);
      }
    }
    if ( j < (_SIZE_OF_OP -1)) {
      val_str.append(", ");
    }
    line_str.append( val_str );
  }
  return line_str;
}
int device_record( const int _dev_index,const int _type )
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  struct timeval tv;
  gettimeofday(&tv,NULL);
  //int ms = tv.tv_usec/1000;
  time_t nSeconds;
  struct tm * pTM;
  time(&nSeconds);
  pTM = localtime(&nSeconds);
  char file_name[128];
  sprintf( file_name,
           "%i-%i-%i.cvs",
           (pTM->tm_year-100),
           (pTM->tm_mon+1),
           pTM->tm_mday );

  // 当设备累计多次启动异常,不再记录
  Device_t* pDev = &Uds_tester::m_sys.devs[ _dev_index ];
  if ( (pDev->booting_stage & _BOOTING_TESTD) == 0 ) {
    return -1;
  }

  char file_path[256];
  sprintf( file_path,"/home/root/Tester/dev_%i/%s",_dev_index,file_name);
  LOGV<<" Recording to: "<<file_path;

  ofstream    fout;
  struct stat file_stat {};
  bool new_file = false;
  if (!stat(file_path, &file_stat)) {
    int  size_kb = file_stat.st_size >> 10;
    if ( size_kb > 2048 ) {
      fout.open(file_path,ofstream::out | ofstream::trunc) ;
      new_file = true;
    } else {
      fout.open(file_path,ofstream::out | ofstream::app ) ;
    }
  } else {
    fout.open(file_path,ofstream::out | ofstream::trunc) ;
    new_file = true;
  }

  if ( !fout.is_open()){
    LOGW<< "The record file was NOT opened!";
    return -1 ;
  } else {
    if ( new_file ) {
      // 新文件,添加首行
      string first_line = head_row();
      fout<<first_line<<endl;
    }
    string line_str = parameter_row( pDev,_type );
    fout<<line_str<<endl;
    fout.close();
  }
  return 0;
  #else
    return 0;
  #endif
#endif
  return 0;
}

void Uds_tester::
profile_record(const int _param)
{
  (void)_param;
  //for ( int i=0;i< ARRAY_SIZE(m_sys.devs);i++ ) {
  LOGI<<" Profile record...";
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    device_record( i,0);
  }
  return ;
}

void Uds_tester::enable_can0_tx(const int _param)
{
  (void)_param;
  LOGI<<" Enable CAN0 transmit..."<<_param;
  Canbus::get_instance()->enable_tx(0,1);
  return ;
}

void Uds_tester::disable_can0_tx(const int _param)
{
  (void)_param;
  LOGI<<" Disable CAN0 transmit..."<<_param;
  Canbus::get_instance()->enable_tx(0,0);
  return ;
}

void Uds_tester::enable_can1_tx(const int _param)
{
  (void)_param;
  LOGI<<" Enable CAN1 transmit..."<<_param;
  Canbus::get_instance()->enable_tx(1,1);
  return ;
}


void Uds_tester::disable_can1_tx(const int _param)
{
  (void)_param;
  LOGI<<" Disable CAN1 transmit..."<<_param;
  Canbus::get_instance()->enable_tx(1,0);
  return ;
}

void Uds_tester::random_poweroff(const int _param )
{
  m_sys.status.random_poweroff_sec = rand() % (_param*10) ;
  LOGI<<" Random poweroff..."<<_param
      <<" sec."<<m_sys.status.random_poweroff_sec;
  return ;
}
// 结束:测试器内部操作
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// 测试器控制接口
void Uds_tester::
set_running( const int _running )
{
  LOGI<<" Running..."<<_running;
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  m_sys.status.running = _running;
  Config_t conf;
  std::memcpy(&conf, &m_sys.config,sizeof(Config_t));

  if (write_data("/home/root/Tester/test.config",
                 reinterpret_cast<char*>(&conf),
                 sizeof(Config_t)) < 0) {
    perror(" Failed to save Config_t");
  }
  return ;
  #else
    return ;
  #endif
#endif
}

void Uds_tester::
set_acc_bat( const int _ctrl )
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH

  int temp = 0;
  if (m_sys.status.acc_bit > 0 ) {
    temp |= (m_sys.status.acc_bit) ;
  } else {
    temp |= _ACC_BITS;
  }

  if (m_sys.status.bat_bit > 0 ) {
    temp |= (m_sys.status.bat_bit) ;
  } else {
    temp |= _BAT_BITS;
  }

  if ( _ctrl > 0 ) {
    m_sys.status.can_switch_io |= temp;
    m_sys.status.acc_status = 1;
    m_sys.status.bat_status = 1;
  } else {
    m_sys.status.can_switch_io &= ~temp;
    m_sys.status.acc_status = 0;
    m_sys.status.bat_status = 0;
  }
  LOGV<<" CAN IO ..."<<m_sys.status.can_switch_io;
  return ;
  #else
    return ;
  #endif
#endif

}

QString Uds_tester::
get_acc_status(void) const 
{
  if ( m_sys.status.acc_status > 0 )
    return QString("ON");
  else 
    return QString("OFF");
}

QString Uds_tester::
get_bat_status(void) const
{
  if ( m_sys.status.bat_status > 0 )
    return QString("ON");
  else 
    return QString("OFF");
}

QString Uds_tester::
get_rand_sec(void) const
{
  float ff = static_cast<float>(m_sys.status.rand_pwr_sec)/10.0;
  return QString("%1 sec").arg( ff,0,'f',1);
}

void Uds_tester::
save_test_config(void)
{
  //Config_t conf;
  //std::memcpy(&conf, &m_sys.config,sizeof(Config_t));
  //if (write_data("/home/root/Spy.config",
  //               reinterpret_cast<char*>(&conf),
  //               sizeof(Config_t)) < 0) {
  //  perror(" Failed to save Config_t");
  //}
  return ;
}

/// 结束:测试器控制接口
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////
/// 控制功能
/// 1. System 参数测试
void Uds_tester::request_cpuid(const int _param)
{
  (void)_param;
  LOGV<<" Request CPU ID:"<<_param;
  request_DID( 0x8100 ) ;
  return;
}

void Uds_tester::request_uptime(const int _param)
{
  (void)_param;
  LOGV<<" Request Uptime:"<<_param;
  request_DID( 0x8101 ) ;
  return;
}

void Uds_tester::request_core_tempeture_0( const int _param )
{
  (void)_param;
  LOGV<<" Request CPU tempture:"<<_param;
  request_DID( 0x8140 ) ;
  return;
}

void Uds_tester::request_loadavg_1min(const int _param)
{
  (void)_param;
  LOGV<<" Request Loadavg:"<<_param;
  request_DID( 0x8110 ) ;
  return;
}

void Uds_tester::request_core_freq_0(const int _param)
{
  (void)_param;
  LOGV<<" Request CPU Freq:"<<_param;
  request_DID( 0x8120 ) ;
  return;
}

void Uds_tester::request_avalid_mem( const int _param )
{
  (void)_param;
  LOGV<<" Request Avalid Memory:"<<_param;
  request_DID( 0x8131 ) ;
  return;
}
void Uds_tester::
request_can0_rx(const int _param)
{
  (void)_param;
  LOGV<<" Request CAN0 Receive counter:"<<_param;
  request_DID( 0x8150 ) ;
  return;
}

void Uds_tester::
request_can0_tx(const int _param)
{
  (void)_param;
  LOGV<<" Request CAN0 transmit counter:"<<_param;
  request_DID( 0x8151 ) ;
  return;
}
void Uds_tester::
check_can(const int _param)
{
  (void)_param;
  LOGV<<" Check CAN0 receive: "<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p = get_param_by_did( 0x8150, pDev) ;
      if ( p != nullptr )  {
        if ( p->val != 0xFFFFFFFF ) {
          if (p->val < _param ) {
            p->exception_flag++;
            pDev->unit_exception |= _ERR_SYS;
            Uds_tester::event_record( QString("Dev[%1] CAN0 Receive(%2) less than (%3)").
                                        arg(i).
                                        arg(p->val).
                                        arg(_param));
          }
        } else {
          LOGW<<"Check Skiped: CAN0 receive counter";
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}


void Uds_tester::request_backlight(const int _param)
{
  (void)_param;
  LOGV<<"Request backlight value:"<<_param;
  request_DID( 0x8142 ) ;
  return;
}

void Uds_tester::set_backlight(const int _param)
{
  (void)_param;
  LOGV<<"Set backlight value:"<<_param;
  m_sys.status.backlight_sta     = 1;
  m_sys.status.brightness_sensor = 0x0000000000000000;
  set_DID( 0x8142,_param) ;
  return;
}

// 直接对比预期状态和传感器反馈状态即可,不考虑ACC或背光控制状态
void Uds_tester::
check_backlight_status(const int _param)
{
  (void)_param;
  long br_temp = m_sys.status.brightness_sensor;
  LOGV<<"Check backlight status:"<<_param;

  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    int br        = 0;// static_cast<int>((br_temp>>(i*2)) & 0x0000000000000003);
    Device_t* dev = &m_sys.devs[i];
    Testing_param_t* pp = get_param_by_did( 0x8143, dev) ;
    if ( pp != nullptr )  {
      br = pp->val;
    }
    if ((dev->booting_stage & _BOOTING_TESTD) != 0 ) {
      if ( _param > 0 ) {
        // Backlight turn ON
        if ( br == 0 ) {
          LOGW<<"The backlight sensor: "<<br
              <<",but target status is "<<_param;
          int bl_val = 0;
          Testing_param_t* p = get_param_by_did( 0x8142, dev) ;
          if ( p != nullptr )  {
            bl_val = p->val;
          }
          //预期为1, 两路传感器均为0,异常
          dev->unit_exception |= _ERR_SYS;
          QString str = QString(" Dev[%1] Check Backlight status is NOT ON" \
                                " [ACC_flag=%2 BL_flag=%3 BlSensor=%4 Req_BL_val=%5]").
                                arg(i).
                                arg(m_sys.status.acc_status).
                                arg(m_sys.status.backlight_sta).
                                arg(br).
                                arg(bl_val);
          Uds_tester::event_record( str );
          capture_image("BacklightNotON");
          pp->exception_flag++;
        }
      } else {
        // Backlight turn OFF
        if ( br == 0x03 ) {
          LOGW<<"The backlight sensor: "<<br
              <<",but target status is "<<_param;
          int bl_val = 0;
          Testing_param_t* p = get_param_by_did( 0x8142, dev) ;
          if ( p != nullptr )  {
            bl_val = p->val;
          }
          //预期为0, 两路传感器均为1,异常
          dev->unit_exception |= _ERR_SYS;
          QString str = QString(" Dev[%1] Check Backlight status is NOT OFF" \
                                " [ACC_flag=%2 BL_flag=%3 BlSensor=%4 Req_BL_val=%5]").
                                arg(i).
                                arg(m_sys.status.acc_status).
                                arg(m_sys.status.backlight_sta).
                                arg(br).
                                arg(bl_val);
          Uds_tester::event_record( str );
          capture_image("BacklightNotOFF");
          pp->exception_flag++;
        }
      }

    }
  }
  return ;
}

/////////////////////////////////////////////////
/// 完成 System 参数

/////////////////////////////////////////////////
/// 2. Orchid 参数测试
void Uds_tester::request_orchid_ignition(const int _param)
{
  (void)_param;
  LOGV<<" Request Ignition from yqsrv_ignitor: "<<_param;
  request_DID( 0x8201 ) ;
  return;
}

// 检查内容:检查反馈ACC状态与以下信号的关系:
// 1. 外部控制
// 2. 背光状态
void Uds_tester::hook_orchid_ignition( struct can_frame* _f)
{
  (void)_f;
  return ;
}

/////////////////////////////////////////////////
/// 完成 Orchid

/////////////////////////////////////////////////
/// Launcher
void Uds_tester::request_launcher_restart(const int _param)
{
  (void)_param;
  LOGV<<" Request Restart Counter from yqsrv_launcher: "<<_param;
  request_DID( 0x8301 ) ;
  return;
}

void Uds_tester::hook_launcher_restart( struct can_frame* _f)
{
  (void)_f;
  return ;
}
void Uds_tester::request_launcher_heartbeat(const int _param)
{
  (void)_param;
  LOGV<<" Request Heartbeat Counter from yqsrv_launcher: "<<_param;
  request_DID( 0x8302 ) ;
  return;
}
/// 完成 Launcher
/////////////////////////////////////////////////

/////////////////////////////////////////////////
///  Hourmeter
void Uds_tester::request_hourmeter_zl_sec(const int _param)
{
  (void)_param;
  LOGV<<" Request Second from yqsrv_hourmeter_ZL: "<<_param;
  request_DID( 0x8402 ) ;
  return;
}
void Uds_tester::request_hourmeter_lv_sec(const int _param)
{
  (void)_param;
  LOGV<<" Request Second from yqsrv_hourmeter_LV: "<<_param;
  request_DID( 0x8401 ) ;
  return;
}
void Uds_tester::request_hourmeter_sec(const int _param)
{
  (void)_param;
  LOGV<<" Request Second from yqsrv_hourmeter: "<<_param;
  request_DID( 0x8400 ) ;
  return;
}
/// 完成 Hourmeter
/////////////////////////////////////////////////


///////////////////////////////////////////////////
/// 4. Kemp 按键盘测试
void Uds_tester::request_kemp_pressed(const int _param)
{
  (void)_param;
  LOGV<<" Request F1 pressed counter from yqsrv_keypad: "<<_param;
  request_DID( 0x8601 ) ;
  return;
}
void Uds_tester::request_kemp_release(const int _param)
{
  (void)_param;
  LOGV<<" Request F1 released counter from yqsrv_keypad: "<<_param;
  request_DID( 0x8602 ) ;
  return;
}
void Uds_tester::
request_kemp_touch_pressed(const int _param)
{
  (void)_param;
  LOGV<<" Request Touch pressed counter from /dev/input/eventX: "<<_param;
    request_DID( 0x8603 ) ;
  return;
}

void Uds_tester::
request_kemp_touch_release(const int _param)
{
  (void)_param;
  LOGV<<" Request Touch release counter from /dev/input/eventX: "<<_param;
  request_DID( 0x8604 ) ;
  return;
}

void Uds_tester::
request_kemp_touch_x(const int _param)
{
  (void)_param;
  LOGV<<" Request Touch position_X from /dev/input/eventX: "<<_param;
  request_DID( 0x8605 ) ;
  return;
}

void Uds_tester::
request_kemp_touch_y(const int _param)
{
  (void)_param;
  LOGV<<" Request Touch position_Y from /dev/input/eventX: "<<_param;
  request_DID( 0x8606 ) ;
  return;
}

void Uds_tester::
set_kemp_touch_counter(const int _param)
{
  (void)_param;
  LOGV<<" Set Touch counter: "<<_param;
  set_DID( 0x8603,_param) ;
  return ;
}

void Uds_tester::
check_touch_mismatch_lt(const int _param)
{
  (void)_param;

  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      int pressed  = 0;
      int released = 0;

      Testing_param_t* pPressed = get_param_by_did(0x8603,pDev);
      Testing_param_t* pRelease = get_param_by_did(0x8604,pDev);

      if ( pPressed != nullptr )  {
        if ( pPressed->val == 0xFFFFFFFF ) {
          pDev->unvalid_check++;
          LOGW<<" Check Skip, Pressed counter is unvalid";
          return ;
        }
      } else {
        LOGW<<" Check Skip,Parameter of Touch Pressed is NULL";
        return ;
      }

      if ( pRelease!= nullptr )  {
        if ( pRelease->val == 0xFFFFFFFF ) {
          LOGW<<" Check Skip, Released counter is unvalid";
          pDev->unvalid_check++;
          return ;
        }
      } else {
        LOGW<<" Check Skip,Parameter of Touch Released is NULL";
        return ;
      }

      pressed = pPressed->val;
      if ( pressed == 0 ) {
        // 当前读取0,首次检查之前没有任何按下输入,标记错误
        pDev->unit_exception |= _ERR_INPUT;
        Uds_tester::event_record( QString("Dev[%1] Touch Pressed is ZERO").arg(i));
      }

      released = pRelease->val;
      if ( released == 0 ) {
        // 当前读取0,首次检查之前没有任何按下输入,标记错误
        pDev->unit_exception |= _ERR_INPUT;
        Uds_tester::event_record(QString("Dev[%1] Touch Released is ZERO")
                                .arg(i));
      }

      int p_r_delta = 0;
      if ( pressed > released ) {
        p_r_delta = pressed - released;
      } else {
        p_r_delta = released - pressed;
      } 
      if ( p_r_delta > _param ) {
        pDev->unit_exception |= _ERR_INPUT;
        LOGW<<" The delta between Pressed and Released is : "<<p_r_delta;
        Uds_tester::event_record( 
                 QString("Dev[%1] Pressed and Released NOT IN PAIRS P=%2 R=%3")
                   .arg(i)
                   .arg(pressed)
                   .arg(released)
                );
      }
    }
  }
  return;
}
// 结束:Kemp测试
///////////////////////////////////////////////////

///////////////////////////////////////////////////
/// 5. MCU 测试
void Uds_tester::request_mcu_fireware(const int _param)
{
  (void)_param;
  LOGV<<"Rquesting MCU fireware major version:"<<_param;
  request_DID( 0x8700 ) ;
  return;
}

void Uds_tester::request_mcu_fireware_minor(const int _param)
{
  (void)_param;
  LOGV<<"Rquesting MCU fireware minor version:"<<_param;
  request_DID( 0x8701 ) ;
  return;
}

void Uds_tester::request_mcu_hardware(const int _param)
{
  (void)_param;
  LOGV<<"Rquesting MCU hardware major version:"<<_param;
  request_DID( 0x8702 ) ;
  return;
}

void Uds_tester::request_mcu_hardware_minor(const int _param)
{
  (void)_param;
  LOGV<<"Rquesting MCU hardware minor version:"<<_param;
  request_DID( 0x8703 ) ;
  return;
}

void Uds_tester::request_mcu_acc_voltage(const int _param)
{
  (void)_param;
  LOGV<<"Rquesting MCU ACC voltage:"<<_param;
  request_DID( 0x8704 ) ;
  return;
}

void Uds_tester::hook_mcu_acc_voltage( struct can_frame* _f)
{
  (void)_f;
  return ;
}

void Uds_tester::request_mcu_bat_voltage(const int _param)
{
  (void)_param;
  LOGV<<"Rquesting MCU Bat voltage:"<<_param;
  request_DID( 0x8705 ) ;
  return;
}

void Uds_tester::
check_acc_voltage_ge(const int _param)
{
  LOGV<<"Check ACC voltage(GE) :"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8704,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          if ( p->val < _param ) {
            pDev->unit_exception |= _ERR_MCU;
            p->exception_flag++;
            Uds_tester::event_record( QString("Dev[%1] MCU ACC voltage %2 less than %3").
                                        arg(i).
                                        arg(p->val).
                                        arg(_param));
          }
        } else {
          LOGW<<"Check Skiped, parameter of acc is unvalid..."<<p->val;
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}
void Uds_tester::
check_bat_voltage_ge(const int _param)
{
  LOGV<<"Check BAT voltage(GE) :"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {

      Testing_param_t* p=get_param_by_did(0x8705,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          if ( p->val < _param ) {
            pDev->unit_exception |= _ERR_MCU;
            p->exception_flag++;
            Uds_tester::event_record( 
              QString("Dev[%1] MCU Battery voltage %2 less than %3").
                arg(i).
                arg(p->val).
                arg(_param)
            );
          }
        } else {
          LOGW<<"Check Skiped, parameter of acc is unvalid..."<<p->val;
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}

void Uds_tester::
check_acc_voltage_lt(const int _param)
{
  LOGV<<"Check ACC voltage(LT) :"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8704,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          if ( p->val > _param ) {
            pDev->unit_exception |= _ERR_MCU;
            p->exception_flag++;

            QString str = QString("Dev[%1] MCU ACC voltage %2 greater than %3")
                            .arg(i)
                            .arg(p->val)
                            .arg(_param);
            Uds_tester::event_record(str);
          }
        } else {
          LOGW<<"Check Skiped, parameter of acc is unvalid..."<<p->val;
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}

void Uds_tester::
check_bat_voltage_lt(const int _param)
{
  LOGV<<"Check BAT voltage(LT) :"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8705,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          if ( p->val > _param ) {
            pDev->unit_exception |= _ERR_MCU;
            p->exception_flag++;
            Uds_tester::event_record( 
                QString("Dev[%1] MCU BAT voltage %2 greater than %3").
                arg(i).
                arg(p->val).
                arg(_param)
            );
          }
        } else {
          LOGW<<"Check Skiped, parameter of acc is unvalid..."<<p->val;
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}
/// 结束:MCU测试
///////////////////////////////////////////////////
///
void Uds_tester::
request_piexl_color( const int _param )
{
  (void)_param;
  LOGV<<"Rquesting color of piexl specialed:"<<_param;
  request_DID( 0x8800 ) ;
  return ;
}

void Uds_tester::
set_lcd_pixel_x(const int _param )
{
  (void)_param;
  LOGV<<"Set position X of piexl:"<<_param;
  set_DID( 0x8801,_param) ;
  return ;
}

void Uds_tester::
set_lcd_pixel_y(const int _param )
{
  (void)_param;
  LOGV<<"Set position Y of piexl:"<<_param;
  set_DID( 0x8802,_param) ;
  return ;
}

void Uds_tester::
check_color_lt(const int _param)
{
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8800,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          double cr = static_cast<double>((p->val & 0x00FF0000) >> 16) ;
          double cg = static_cast<double>((p->val & 0x0000FF00) >> 8);
          double cb = static_cast<double>((p->val & 0x000000FF));

          double cc = pow(cr,2);
          double gg = pow(cg,2);
          double bb = pow(cb,2);
          int radius = static_cast<int>(sqrt(cc+gg+bb));

          LOGI<<"Dev["<<i<<"] Check color radius < "<<_param
              <<" RGB{"<<cr<<","<<cg<<","<<cb
              <<"} radius:"<<radius;
          if ( radius >= _param ) {
            int br = 0xFF;
            Testing_param_t* pp=get_param_by_did(0x8143,pDev);
            if ( pp != nullptr ) {
              br = pp->val;
            }
            // 颜色大于预期,异常
            pDev->unit_exception |= _ERR_SYS;
            p->exception_flag++;
            Uds_tester::event_record(
              QString("Dev[%1] Color_LT (%2,%3,%4)radius(%5) > %6,bl.%7")
                .arg(i)
                .arg(cr)
                .arg(cg)
                .arg(cb)
                .arg(radius)
                .arg(_param)
                .arg(br)
              );
            LOGW<<"The color radius("<<radius<<") of piexl is greater "
                <<" than "<<_param
                <<" backlight sensor."<<br;
            capture_image("ColorOfVideo_LT");
          } else {
            // 颜色小于预期,正常
          }
        } else {
          LOGW<<"Check Skiped,the parameter is unvalid";
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}
void Uds_tester::
check_color_ge(const int _param)
{
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8800,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          double cr = static_cast<double>((p->val & 0x00FF0000) >> 16) ;
          double cg = static_cast<double>((p->val & 0x0000FF00) >> 8);
          double cb = static_cast<double>((p->val & 0x000000FF));

          double cc = pow(cr,2);
          double gg = pow(cg,2);
          double bb = pow(cb,2);
          int radius = static_cast<int>(sqrt(cc+gg+bb));

          LOGI<<"Dev["<<i<<"] Check color radius > "<<_param
              <<" RGB{"<<cr<<","<<cg<<","<<cb
              <<"} radius:"<<radius;
          if ( radius < _param ) {
            int br = 0xFF;
            Testing_param_t* pp=get_param_by_did(0x8143,pDev);
            if ( pp != nullptr ) {
              br = pp->val;
            }
            // 颜色小于预期,异常
            pDev->unit_exception |= _ERR_SYS;
            p->exception_flag++;
            Uds_tester::event_record(
              QString("Dev[%1] Color_GE (%2,%3,%4)radius(%5) < %6,bl.%7")
                .arg(i)
                .arg(cr)
                .arg(cg)
                .arg(cb)
                .arg(radius)
                .arg(_param)
                .arg(br)
              );
            LOGW<<"The color radius("<<radius<<") of piexl is letter"
                <<" than "<<_param
                <<" backlight sensor "<<br;
            capture_image("ColorOfVideo_GE");
          } else {
            // 颜色大于等于预期,正常
          }
        } else {
          LOGW<<"Check Skiped,the parameter is unvalid";
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}

void Uds_tester::
check_color_change(const int _param)
{
  LOGV<<"Checking color of piexl:"<<_param;
  //set_DID( 0x8802,_param) ;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8800,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          int cr = (p->val & 0x00FF0000) >> 16;
          int pr = (pDev->pre_color & 0x00FF0000) >> 16;
          int dr = abs( cr - pr );

          int cg = (p->val & 0x0000FF00) >> 8;
          int pg = (pDev->pre_color & 0x0000FF00) >> 8;
          int dg = abs( cg - pg );

          int cb = (p->val & 0x000000FF);
          int pb = (pDev->pre_color & 0x000000FF);
          int db = abs( cb - pb );

          if ( (dr<=_param) && 
               (db<=_param) && 
               (dg<=_param) ) {
            // 颜色相同,累计计数判断
            if ( pDev->color_unchanged_cnt >= m_sys.status.max_color_unchanged_cnt ) {
              // 颜色的改变量小于阈值
              pDev->unit_exception |= _ERR_SYS;
              p->exception_flag++;
              Uds_tester::event_record(
                    QString("Dev[%1] Color value Changed (%2,%3) less than %4")
                      .arg(i)
                      .arg(p->val)
                      .arg(pDev->pre_color)
                      .arg(_param)
                    );
            }
            pDev->color_unchanged_cnt++;
            LOGW<<"Dev["<<i<<"] Color UNCHANGED("<<_param<<") Pre:"<<pDev->pre_color<<" Cur:"<<p->val;
          } else {
            pDev->color_unchanged_cnt = 0;
          }
          pDev->pre_color = p->val;
        } else {
          LOGW<<"Check Skiped,the parameter is unvalid";
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}

///////////////////////////////////////////////////
/// RTC 测试
void Uds_tester::
request_rtc_year(const int _param)
{
  (void)_param;
  LOGI<<" Request RTC year:"<<_param;
  request_DID( 0x8900 ) ;
  return;
}
void Uds_tester::
request_rtc_month(const int _param)
{
  (void)_param;
  LOGI<<" Request RTC month:"<<_param;
  request_DID( 0x8901 ) ;
  return;
}
void Uds_tester::
request_rtc_day(const int _param)
{
  (void)_param;
  LOGI<<" Request RTC day:"<<_param;
  request_DID( 0x8902 ) ;
  return;
}
void Uds_tester::
request_rtc_hour(const int _param)
{
  (void)_param;
  LOGV<<" Request RTC hour:"<<_param;
  request_DID( 0x8903 ) ;
  return;
}
void Uds_tester::
request_rtc_min(const int _param)
{
  (void)_param;
  LOGV<<" Request RTC minute:"<<_param;
  request_DID( 0x8904 ) ;
  return;
}
void Uds_tester::
request_rtc_sec(const int _param)
{
  (void)_param;
  LOGV<<" Request RTC second:"<<_param;
  request_DID( 0x8905 ) ;
  return;
}
void Uds_tester::
check_rtc_year(const int _param)
{
  LOGI<<"Checking RTC Year:"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8900,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          if (p->val != _param) {
            pDev->unit_exception |= _ERR_RTC;
            p->exception_flag++;
            Uds_tester::event_record(
                  QString("Dev[%1] Read RTC year %2,Not equal to target.%3")
                    .arg(i)
                    .arg(p->val)
                    .arg(_param)
                  );
            LOGW<<"Dev["<<i
                <<"] Read RTC Year."<<p->val
                <<" NOT Equal to target."<<_param;
          }
        } else {
          LOGW<<"Check Skiped,the parameter is unvalid";
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}
void Uds_tester::
check_rtc_month(const int _param)
{
  LOGI<<"Checking RTC Month:"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8901,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          if (p->val != _param) {
            pDev->unit_exception |= _ERR_RTC;
            p->exception_flag++;
            Uds_tester::event_record(
                  QString("Dev[%1] Read RTC Month.%2,Not equal to target.%3")
                    .arg(i)
                    .arg(p->val)
                    .arg(_param)
                  );
            LOGW<<"Dev["<<i
                <<"] Read RTC Month."<<p->val
                <<" NOT Equal to target."<<_param;
          }
        } else {
          LOGW<<"Check Skiped,the parameter is unvalid";
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}
void Uds_tester::
check_rtc_day(const int _param)
{
  LOGI<<"Checking RTC Day:"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev = &m_sys.devs[i];
    if ((pDev->booting_stage & _BOOTING_TESTD) != 0 ) {
      Testing_param_t* p=get_param_by_did(0x8902,pDev);
      if ( p != nullptr ) {
        if ( p->val != 0xFFFFFFFF ) {
          if (p->val != _param) {
            pDev->unit_exception |= _ERR_RTC;
            p->exception_flag++;
            Uds_tester::event_record(
                  QString("Dev[%1] Read RTC Day.%2,Not equal to target.%3")
                    .arg(i)
                    .arg(p->val)
                    .arg(_param)
                  );
            LOGW<<"Dev["<<i
                <<"] Read RTC Day."<<p->val
                <<" NOT Equal to target."<<_param;
          }
        } else {
          LOGW<<"Check Skiped,the parameter is unvalid";
          pDev->unvalid_check++;
        }
      }
    }
  }
  return ;
}
/// 完成 RTC 测试
///////////////////////////////////////////////////

///////////////////////////////////////////////////
/// 通用控制
void Uds_tester::
set_device_number(const int _param )
{
  LOGV<<"Set the number of devices tested"<<_param;
  m_sys.sizeof_dev_tested = _param;
  return ;
}
/// 1. 电源控制
void Uds_tester::
set_acc_bit(const int _param )
{
  LOGV<<"Set ACC bit-mask of CAN IO"<<_param;
  m_sys.status.acc_bit = _param;
  return ;
}

void Uds_tester::
set_bat_bit(const int _param )
{
  LOGV<<"Set BAT bit-mask of CAN IO"<<_param;
  m_sys.status.bat_bit = _param;
  return ;
}
void Uds_tester::
enable_quit_exception(const int _param )
{
  LOGV<<"Enable QUIT when exception occur"<<_param;
  m_sys.status.enable_exception_quit = _param;
  return ;
}
void Uds_tester::
set_max_color_changed_cnt(const int _param )
{
  LOGV<<"Set Max exception of color unchanged"<<_param;
  m_sys.status.max_color_unchanged_cnt = _param;
  return ;
}

// cansend can1 -i 0x00AA0101 -e 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
void Uds_tester::poweron_battery(const int _param)
{
  (void)_param;
  LOGV<<"Turn Battery ON"<<_param;
  m_sys.status.bat_status = 1;
  if (m_sys.status.bat_bit >0 ) {
    m_sys.status.can_switch_io |= m_sys.status.bat_bit;
  } else {
    m_sys.status.can_switch_io |= _BAT_BITS;
  }
  return ;
}

void Uds_tester::poweron_ignition(const int _param)
{
  (void)_param;
  LOGV<<"Turn ACC ON "<<_param;
  m_sys.status.acc_status     = 1;

  if (m_sys.status.acc_bit >0 ) {
    m_sys.status.can_switch_io |= m_sys.status.acc_bit;
  } else {
    m_sys.status.can_switch_io |= _ACC_BITS;
  }

  return ;
}

void Uds_tester::poweroff_ignition(const int _param )
{
  (void)_param;
  LOGV<<"Turn ACC OFF "<<_param;
  m_sys.status.acc_status         = 0;
  m_sys.status.brightness_sensor  = 0xFFFFFFFFFFFFFFFF;

  if (m_sys.status.acc_bit >0 ) {
    m_sys.status.can_switch_io &= (~m_sys.status.acc_bit) ;
  } else {
    m_sys.status.can_switch_io &= (~_ACC_BITS );
  }
  return ;
}

void Uds_tester::poweroff_battery( const int _param )
{
  (void)_param;
  LOGV<<"Turn BAT OFF "<<_param;
  m_sys.status.bat_status = 0;

  if (m_sys.status.bat_bit >0 ) {
    m_sys.status.can_switch_io &= (~m_sys.status.bat_bit) ;
  } else {
    m_sys.status.can_switch_io &= (~_BAT_BITS);
  }
  return ;
}

void Uds_tester::
check_booting(const int _param)
{
  (void)_param;
  LOGV<<"Checking the boot status"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev     = &m_sys.devs[i];

    Testing_param_t* p = nullptr;
    // 以下三个标记均来自 receive_ack() 填入
    p = get_param_by_did(0x8003,pDev);
    if ( p != nullptr) {
      if ((pDev->booting_stage & _BOOTING_1CF0 ) == 0 ) {
        pDev->unit_exception |= _ERR_SYS;
        p->exception_flag++;
        Uds_tester::event_record(
              QString("Dev[%1] System boot failed (1CF0)").arg(i));
      }
    }

    p = get_param_by_did(0x8004,pDev);
    if ( p != nullptr) {
      if ((pDev->booting_stage & _BOOTING_1CF1 ) == 0 ) {
        pDev->unit_exception |= _ERR_SYS;
        p->exception_flag++;
        Uds_tester::event_record(
              QString("Dev[%1] System boot failed (1CF1)").arg(i));
      }
    }

    p = get_param_by_did(0x8005,pDev);
    if ( p != nullptr) {
      if ((pDev->booting_stage & _BOOTING_TESTD ) == 0 ) {
        pDev->unit_exception |= _ERR_SYS;
        p->exception_flag++;
        Uds_tester::event_record(
              QString("Dev[%1] yqsrv_testd boot failed").arg(i));
      }
    }
  }
  return ;
}
///////////////////////////////////////////////////
/// 2. 其他控制
void Uds_tester::after_poweroff( const int _param )
{
  int reserve[5];
  (void)_param;
  LOGV<<"Post process of test"<<_param;
  for ( int i=0;i< m_sys.sizeof_dev_tested;i++ ) {
    Device_t* pDev     = &m_sys.devs[i];
    Testing_param_t* p = nullptr;

    p = get_param_by_did(0x8001,pDev);
    if ( p != nullptr) {
      if ((pDev->booting_stage & (_BOOTING_1CF0 | 
                                  _BOOTING_1CF1 | 
                                  _BOOTING_TESTD)) != 0 ) {
        p->val++;
      }
    }

    p = get_param_by_did(0x8006,pDev);
    if ( p != nullptr) {
      p->val = pDev->negitive_responed_cnt;
    }
    p = get_param_by_did(0x8007,pDev);
    if ( p != nullptr) {
      p->val = pDev->unvalid_check;
    }
    p = get_param_by_did(0x8008,pDev) ;
    if ( p != nullptr) {
      p->val = 0xFFFFFFFF;
    }

    pDev->pre_touch_pressed = 0;
    pDev->pre_touch_release = 0;
    pDev->pre_color         = 0;
    pDev->booting_stage     = 0;
    pDev->color_unchanged_cnt = 0;
  }
  m_sys.status.preoid_cycle++;
  m_sys.total_cycle++;
  return ;
}
/// 结束:通用控制

/// end of code ///
