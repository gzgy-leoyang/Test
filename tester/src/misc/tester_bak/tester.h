#ifndef TESTER_H
#define TESTER_H

#include <iostream>
#include <map>
#include <iterator>
#include <cstring>
#include <vector>

// 打印提示
#define _DEBUG (0)

#define _TEST_ENABLE_SYS          (0x00000001)
#define _TEST_ENABLE_ORCHID       (0x00000002)
#define _TEST_ENABLE_HOURMETER_ZL (0x00000004)
#define _TEST_ENABLE_HOURMETER_LV (0x00000008)
#define _TEST_ENABLE_HOURMETER    (0x00000010)
#define _TEST_ENABLE_LAUNCHER     (0x00000020)
#define _TEST_ENABLE_KEMP         (0x00000040)
#define _TEST_ENABLE_CAN0         (0x00000080)
#define _TEST_ENABLE_CAN1         (0x00000100)
#define _TEST_ENABLE_RANDOM_PWR   (0x00000200)
#define _TEST_ENABLE_MCU          (0x00000400)

// **必须修改,添加序号和错误位**
#define _OP_SYS_BOOT_COUNT   (0                     )
#define _OP_SYS_REQ_TIMEOUE  (_OP_SYS_BOOT_COUNT  +1)
#define _OP_SYS_1CF0         (_OP_SYS_REQ_TIMEOUE +1)
#define _OP_SYS_1CF1         (_OP_SYS_1CF0        +1)
#define _OP_SYS_TESTD_BOOTED (_OP_SYS_1CF1        +1)
#define _OP_SYS_CPUID        (_OP_SYS_TESTD_BOOTED+1)
#define _OP_SYS_UPTIME       (_OP_SYS_CPUID       +1)
#define _OP_SYS_CORE_TEMP    (_OP_SYS_UPTIME      +1)
#define _OP_SYS_LOADAVG      (_OP_SYS_CORE_TEMP   +1)
#define _OP_SYS_CORE_FREQ    (_OP_SYS_LOADAVG     +1)
#define _OP_SYS_AVALIMEM     (_OP_SYS_CORE_FREQ   +1)
#define _OP_ORCHID_ACC       (_OP_SYS_AVALIMEM    +1)
#define _OP_HYMAN_X_SEC      (_OP_ORCHID_ACC      +1)
#define _OP_HYMAN_LV_SEC     (_OP_HYMAN_X_SEC     +1)
#define _OP_HYMAN_ZL_SEC     (_OP_HYMAN_LV_SEC    +1)
#define _OP_LAUNCHER_RESTART (_OP_HYMAN_ZL_SEC    +1)
#define _OP_LAUNCHER_HEART   (_OP_LAUNCHER_RESTART+1)
#define _OP_KEMP_F1_PRESSED  (_OP_LAUNCHER_HEART  +1)
#define _OP_KEMP_F1_RELEASE  (_OP_KEMP_F1_PRESSED +1)
#define _OP_MCU_FIREWARE     (_OP_KEMP_F1_RELEASE +1)
#define _OP_MCU_HARDWARE     (_OP_MCU_FIREWARE    +1)
#define _OP_MCU_ACC_VOLTAGE  (_OP_MCU_HARDWARE    +1)
#define _OP_MCU_BAT_VOLTAGE  (_OP_MCU_ACC_VOLTAGE +1)

#define _ERR_SYS_BOOT_COUNT   (1<<_OP_SYS_BOOT_COUNT )
#define _ERR_SYS_REQ_TIMEOUT  (1<<_OP_SYS_REQ_TIMEOUE)
#define _ERR_SYS_1CF0         (1<<_OP_SYS_1CF0)
#define _ERR_SYS_1CF1         (1<<_OP_SYS_1CF1)
#define _ERR_SYS_TESTD_BOOTED (1<<_OP_SYS_TESTD_BOOTED)
#define _ERR_SYS_CPUID        (1<<_OP_SYS_CPUID      )
#define _ERR_SYS_UPTIME       (1<<_OP_SYS_UPTIME     )
#define _ERR_SYS_CORE_TEMP    (1<<_OP_SYS_CORE_TEMP  )
#define _ERR_SYS_LOADAVG      (1<<_OP_SYS_LOADAVG    )
#define _ERR_SYS_CORE_FREQ    (1<<_OP_SYS_CORE_FREQ  )
#define _ERR_SYS_AVALIMEM     (1<<_OP_SYS_AVALIMEM   )
#define _ERR_ORCHID_ACC       (1<<_OP_ORCHID_ACC     )
#define _ERR_HYMAN_X_SEC      (1<<_OP_HYMAN_X_SEC    )
#define _ERR_HYMAN_LV_SEC     (1<<_OP_HYMAN_LV_SEC   )
#define _ERR_HYMAN_ZL_SEC     (1<<_OP_HYMAN_ZL_SEC   )
#define _ERR_LAUNCHER_RESTART (1<<_OP_LAUNCHER_RESTART)
#define _ERR_LAUNCHER_HEART   (1<<_OP_LAUNCHER_HEART )
#define _ERR_KEMP_F1_PRESSED  (1<<_OP_KEMP_F1_PRESSED)
#define _ERR_KEMP_F1_RELEASE  (1<<_OP_KEMP_F1_RELEASE)
#define _ERR_MCU_FIREWARE     (1<<_OP_MCU_FIREWARE   )
#define _ERR_MCU_HARDWARE     (1<<_OP_MCU_HARDWARE   )
#define _ERR_MCU_ACC_VOLTAGE  (1<<_OP_MCU_ACC_VOLTAGE)
#define _ERR_MCU_BAT_VOLTAGE  (1<<_OP_MCU_BAT_VOLTAGE)

// **必须修改,添加高等级错误标志**
// 分类错误,在记录文件是区分,其余非致命错误为警告级别
#define _FATAL_ERROR          (_ERR_SYS_BOOT_COUNT  | \
                               _ERR_ORCHID_ACC      | \
                               _ERR_HYMAN_X_SEC     | \
                               _ERR_HYMAN_ZL_SEC    | \
                               _ERR_HYMAN_LV_SEC    | \
                               _ERR_MCU_ACC_VOLTAGE | \
                               _ERR_MCU_BAT_VOLTAGE )

// **必须修改**
#define _SIZE_OF_OP   (25)  // 测试参数**
#define _SIZE_OF_DEVS (5)   // 测试设备**
                            //
#define GRANULES               (100)           // 时间颗粒,10ms
#define PROCESS_GRANULES       (GRANULES*1000)// 测试循环的时间颗粒
#define ACTIVE_POINT_MS(ms)    (ms/GRANULES)  // 动作点的时刻

// 启动过程标记
#define _BOOTING_1CF0  (0x00000001)
#define _BOOTING_1CF1  (0x00000002)
#define _BOOTING_TESTD (0x00000004)

// 外部CAN开关控制位
#define _ACC_BITS      ( 0x0F )
#define _IGNITION_BITS ( 0xF0 )

extern int   init_test_sys();
extern void  receive_ack( struct can_frame* _frame );
extern void* thread_handler(void* arg);
extern void info( const char* _function,const char* _msg );
extern int  request_DID( const int _did ) ;
extern int insert_op( std::string& _op,int _msec,int _rep );


typedef void (*func_ptr)(void);
typedef void (*Post_hook)(struct can_frame* _f);
typedef void (*Active_hook)(void);




// "时刻-动作函数",在指定时刻执行指定函数
typedef struct {
  int         sec;
  Active_hook active;
} Time_active_t;

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
  int      req_pending_cnt; // 发出请求,挂起计数
  int      test_normal;     // 0:正常, >0:异常
  int      responsed;       // 0, 未发出; 1,已发出,等待回复
  int      resp_timeout;    // 等待回复时间
  unsigned long exception_flag;     // 当前异常记录
  unsigned long pre_exception_flag; // 历史异常记录
  UDS_op_t op_list[ _SIZE_OF_OP ];  // 查询参数列表
  int      booting_stage;
  int      delta_of_boot;
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
  int running;
  int cur_did;
  int acc_status;    // 钥匙开关控制状态
  int bat_status;    // 常电控制状态
  int rand_pwr_sec;  // 当前随机运行剩余秒
  int preoid_cycle;
  int preoid_second;
  int can_switch_io;    // 保存外部CAN IO模块的控制命令
  int can_tx_fps_0; // CAN0总线TX帧率
  int can_rx_fps_0; // CAN0总线RX帧率
  int can_tx_fps_1; // CAN1总线RX帧率
  int can_rx_fps_1; // CAN1总线TX帧率
  double can_load_0;    // CAN0总线负载率
  double can_load_1;    // CAN1总线负载率
} Ctrl_status_t;

typedef struct {
  int           total_cycle;
  int           total_second;
  Ctrl_status_t status;
  Config_t      config;
  Device_t      devs[_SIZE_OF_DEVS];
} Test_sys_t;

extern Test_sys_t  m_sys;

#endif //UDS_TESTER_H 
