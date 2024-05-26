#ifndef UDS_TESTER_H
#define UDS_TESTER_H

#include "canbus.h"

#include <unistd.h>
#include <pthread.h>
#include <QTimer>

#include <iostream>
#include <string>

// **必须修改**
#define _SIZE_OF_OP   (44)  // 测试参数**
#define _SIZE_OF_DEVS (16)   // 测试设备**
// 打印提示
#define _DEBUG (1)

#define _ERR_SYS           ( 0x00000001 )
#define _ERR_SRV_IGNITION  ( 0x00000002 )
#define _ERR_SRV_LAUNCHER  ( 0x00000004 )
#define _ERR_SRV_HOURMETER ( 0x00000008 )
#define _ERR_INPUT         ( 0x00000010 )
#define _ERR_MCU           ( 0x00000020 )
#define _ERR_RTC           ( 0x00000040 )

#define GRANULES               (100)           // 时间颗粒,10ms
#define PROCESS_GRANULES       (GRANULES*1000)// 测试循环的时间颗粒
#define ACTIVE_POINT_MS(ms)    (ms/GRANULES)  // 动作点的时刻

// 启动过程标记
#define _BOOTING_1CF0  (0x00000001)
#define _BOOTING_1CF1  (0x00000002)
#define _BOOTING_TESTD (0x00000004)

// 外部CAN开关控制位
#define _ACC_BITS ( 0x01 )
#define _BAT_BITS ( 0x02 )

extern int uds_tester_step_index;

class Uds_tester;

typedef void (Uds_tester::*func_ptr)(void);
typedef void (Uds_tester::*Post_hook)(struct can_frame* _f);
typedef void (*Active_hook)(const int);


// "时刻-动作函数",在指定时刻执行指定函数
typedef struct {
  int         sec;
  Active_hook active;
  int         param;
} Time_active_t;

///  测试过程数据结构定义
typedef struct {
  char      desp[128];
  int       did;
  int       result;
  int       saft_acces;
  int       session_type;
  float     rate;
  Post_hook hook;
} UDS_op_t;

typedef struct {
  int val;
  int exception_flag;
} Testing_param_t;

typedef struct {
  int            req_pending_cnt;       // 发出请求,挂起计数
  int            test_normal;           // 0:正常, >0:异常
  int            responsed;             // 0, 未发出; 1,已发出,等待回复
  int            resp_timeout;          // 等待回复时间
  unsigned long  exception_flag;        // 当前异常记录
  unsigned long  pre_exception_flag;    // 历史异常记录
  //UDS_op_t       op_list[ _SIZE_OF_OP ];// 查询参数列表
  int            booting_stage;
  int            delta_of_boot;

  // 记录前一次触摸参数,用于当前比较
  int            pre_touch_pressed;
  int            pre_touch_release;
  int            pre_touch_x;
  int            pre_touch_y;

  /// 新结构(异常标记+数据),不再包含参数名称等公共的字段属性
  int            unit_exception; // 记录32个单元的异常标记
  int            exception[32];  // 每个单元由32个异常标记位组成
  //int            test_val[32][256];

  Testing_param_t val_matrix[32][256];

  int            pre_color;
  int            color_unchanged_cnt;
  int            negitive_responed_cnt; // 收到NRC的计数,来自yqsrv_testd
  int            unvalid_check;// 本地无法完成check的计数
} Device_t;

typedef struct {
  int test_enable;      // 测试使能控制
  int max_rand_pwr_sec; // 设置最大随机秒
  int auto_start;       // 根据配置自动开始测试
  int can0_load;    // CAN0发送间隔
  int can1_load;    // CAN1发送间隔
  int rand_pwr_pressure;// 随机开关最大时间间隔
} Config_t;

typedef struct {
  int    running;
  int    cur_did;
  int    acc_status;    // 钥匙开关控制状态
  int    bat_status;    // 常电控制状态
  int    backlight_sta; // 背光的控制状态,对比读取亮度值和传感器反馈
  int    rand_pwr_sec;  // 当前随机运行剩余秒
  int    preoid_cycle;
  int    preoid_second;
  int    can_switch_io;    // 保存外部CAN IO模块的控制命令
  int    can_tx_fps_0; // CAN0总线TX帧率
  int    can_rx_fps_0; // CAN0总线RX帧率
  int    can_tx_fps_1; // CAN1总线RX帧率
  int    can_rx_fps_1; // CAN1总线TX帧率
  double can_load_0;    // CAN0总线负载率
  double can_load_1;    // CAN1总线负载率
  long   brightness_sensor;
  int    process_step_index;
  int    total_step;
  int    next_step_sec;
  int    random_poweroff_sec;
  int    acc_bit;
  int    bat_bit;
  int    tick_counter;
  int    enable_exception_quit;
  int    max_color_unchanged_cnt;
  int    tick_max;
} Ctrl_status_t;


typedef struct {
  int           total_cycle;       // 累计测试循环计数
  int           total_second;      // 累计测试秒计数
  int           sizeof_dev_tested; // 当前被测试设备数量
  Ctrl_status_t status;
  Config_t      config;
  Device_t      devs[_SIZE_OF_DEVS];
  std::string   config_file_name;
} Test_sys_t;

  /// 记录数据结构定义完
class Uds_tester: public QObject
{

  Q_OBJECT
public:
  static Uds_tester* get_instance(void){ return m_tester; }
  void set_running( const int _running );
  void set_acc_bat( const int _ctrl );
  QString get_acc_status(void) const ;
  QString get_bat_status(void) const ;
  QString get_rand_sec(void) const ;

  void save_test_config(void);

  //////////////////////////////
  // 完整的测试环境描述
  static Test_sys_t  m_sys;
  static UDS_op_t    dev_op[] ;
  //////////////////////////////
  /// 0. 电源控制
  static void poweron_battery(const int _param);
  static void poweron_ignition(const int _param);
  static void poweroff_ignition(const int _param);
  static void poweroff_battery(const int _param);
  static void check_booting(const int _param);
  /// 1. System
  static void request_cpuid(const int _param);
  static void request_uptime(const int _param);
  static void request_core_tempeture_0(const int _param);
  static void request_loadavg_1min(const int _param);
  static void request_core_freq_0(const int _param);
  static void request_avalid_mem(const int _param);
  static void request_can0_rx(const int _param);
  static void request_can0_tx(const int _param);
  static void check_can(const int _param);
  
  static void set_backlight(const int _param);
  static void request_backlight(const int _param);
  static void request_brightnerr_sensor(const int _param);
  //static void check_backlight(const int _param);
  static void check_backlight_status(const int _param);
  /// 2. Orchid
  static void request_orchid_ignition(const int _param);
  static void hook_orchid_ignition( struct can_frame* _f);
  // hourmeter
  static void request_hourmeter_zl_sec(const int _param);
  static void request_hourmeter_lv_sec(const int _param);
  static void request_hourmeter_sec(const int _param);
  // launcher
  static void request_launcher_restart(const int _param);
  static void hook_launcher_restart( struct can_frame* _f);
  static void request_launcher_heartbeat(const int _param);
  // kemp
  static void request_kemp_pressed(const int _param);
  static void request_kemp_release(const int _param);
  static void request_kemp_touch_pressed(const int _param);
  static void request_kemp_touch_release(const int _param);
  static void request_kemp_touch_x(const int _param);
  static void request_kemp_touch_y(const int _param);
  static void check_touch_mismatch_lt(const int _param);
  static void set_kemp_touch_counter(const int _param);
  // mcu
  static void request_mcu_fireware(const int _param);
  static void request_mcu_fireware_minor(const int _param);
  static void request_mcu_hardware(const int _param);
  static void request_mcu_hardware_minor(const int _param);
  static void request_mcu_acc_voltage(const int _param);
  static void request_mcu_bat_voltage(const int _param);
  static void hook_mcu_acc_voltage( struct can_frame* _f );
  //static void check_mcu(const int _param);

  static void request_rtc_year(const int _param);
  static void request_rtc_month(const int _param);
  static void request_rtc_day(const int _param);
  static void request_rtc_hour(const int _param);
  static void request_rtc_min(const int _param);
  static void request_rtc_sec(const int _param);
  static void check_rtc_year(const int _param);
  static void check_rtc_month(const int _param);
  static void check_rtc_day(const int _param);
  
  static void request_piexl_color(const int _param);

  static void check_acc_voltage_ge(const int _param);
  static void check_bat_voltage_ge(const int _param);

  static void check_acc_voltage_lt(const int _param);
  static void check_bat_voltage_lt(const int _param);

  static void check_color_change(const int _param);
  static void check_color_ge(const int _param);
  static void check_color_lt(const int _param);
  //int rand_pwr_ctrl (const int _param) ;
  static void after_poweroff(const int _param) ;

  static void enable_can0_tx(const int _param) ;
  static void enable_can1_tx(const int _param) ;
  static void disable_can0_tx(const int _param) ;
  static void disable_can1_tx(const int _param) ;
  static void random_poweroff(const int _param) ;

  static void set_acc_bit(const int _param );
  static void set_bat_bit(const int _param );
  static void set_device_number(const int _param );
  static void set_lcd_pixel_x(const int _param );
  static void set_lcd_pixel_y(const int _param );
  static void enable_quit_exception(const int _param );
  static void set_max_color_changed_cnt(const int _param );
  //////////////////////////////
  /// "时机-动作"序列
  // 自定义
  static std::vector< Time_active_t > actives;
  static std::map< std::string, Active_hook > name_activer_map;
  // 基本
  // 静态,用于线程的成员函数
  static void* thread_handler(void* arg);
  static void  receive_ack( struct can_frame* _frame );
  static void  set_can_switch ( const int _io );
  static int   request_DID( const int _did ) ;
  static int   load_config( std::string& _file );
  static int   insert_op( std::string& _op,int _msec,int _rep );
  static int   set_DID(const int _did,const int _val) ;
  //static void  info( const char* _function,const char* _msg );
  static void capture_image(const char* _tips);

  static void  event_record( const QString& _str);
  static int   stats_record(void);
  static void  profile_record(const int _param);

  static Testing_param_t* get_param_by_did( int _did,
                               Device_t* _dev);
  static int get_val_except_by_did( int _did,Device_t* _dev,int * _val,int* _except);
  static int set_value_by_did( int _did,Device_t* _dev,int _val );
  int   init_test_sys();
  //////////////////////////////

private:
  explicit Uds_tester(QObject *parent = nullptr);

  int read_data(const char* _file_name,
                char* _buf,
                const int _size );

  int write_data(const char* _file_name,
                 char *_buf,
                 const int _size);
  // 单例,全局唯一
  static Uds_tester* m_tester;
  static pthread_t   m_pThread;
  static Active_hook m_act;
};




#endif //UDS_TESTER_H 
