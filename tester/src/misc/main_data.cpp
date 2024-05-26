#include "main_data.h"
#include <QDateTime>
#include <QDebug>

#include <iostream>
#include <fstream>
#include <chrono>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

#include "main.h"
//#include "tester/tester.h"
/*
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  #else
  #endif
#endif
*/

using namespace std;

int get_can_rx( const int _port,int* _val );
int get_can_tx( const int _port,int* _val );

int read_data( const char* _file_name, 
                     char* _buf,
               const int _size );
int write_data( const char* _file_name ,
                      char *_buf,
                const int _size );


// 用 dev_op 初始化所有设备的 op_list 结构
Main_data::Main_data(QObject *parent)
    : QObject{parent}
{
  Uds_tester* test = Uds_tester::get_instance();
  // 填充第一行内容
  m_menu_index = 0;
  m_model = new Data_model(this);

  m_model->pushData(new Data_element(QString("状态"),
                                            QString("#2b2a4c"),
                                            QString("测试中"),
                                            QString("#2b2a4c"),
                                            this));

  m_model->pushData(new Data_element(QString("动作序列"),
                                            QString("#2b2a4c"),
                                            QString(""),
                                            QString("#2b2a4c"),
                                            this));
  m_model->refresh_highlight( m_menu_index,"#ea906c");

  // 填充后续设备行内容
  //Test_sys_t* pSys = &Uds_tester::m_sys;
  // 初始化时,按32设备准备模型,后续模型中
  // 实际刷新数据过程,按脚本命令配置参数控制
  // 所以,在脚本加载之前,该列表不会实际刷新模型
  for( int i=0;i<_SIZE_OF_DEVS;i++ ) {
    Device_t* pDev = &Uds_tester::m_sys.devs[i];
    m_model->pushData(new Data_element(QString("设备[%1]").arg(i+1),
                                              QString("#2b2a4c"),
                                              QString("----"),
                                              QString("#2b2a4c"),
                                              this));
  }
  m_devs_record_preoid_in_sec = 30;
  m_test_record_preoid_in_sec = 30;
  m_run        = Uds_tester::m_sys.status.running;
  m_menu_index = 0;
  // 1s更新一次左侧列表
  m_tim        = new QTimer(this);
  connect(m_tim,SIGNAL(timeout()),this,SLOT(timeout_slot()));
  m_tim->start(1000);
}

void Main_data::
timeout_slot()
{
  // 更新左侧列表中,各设备的请求应答情况
  Uds_tester* test = Uds_tester::get_instance();
  Test_sys_t* pSys = &Uds_tester::m_sys;

  static int tick = 0;
  if ( m_run > 0 ) {
    tick++;
    if ( (tick%2)==0 )
      emit run_tick( 1,pSys->status.acc_status,pSys->status.bat_status );
    else 
      emit run_tick( 0,pSys->status.acc_status,pSys->status.bat_status );
  }

  m_model->refresh( 1,
                    QString("%1ms [%2:%3]")
                          .arg(pSys->status.next_step_sec*100)
                          .arg(pSys->status.tick_counter/10)
                          .arg(pSys->status.tick_max));

  for( int i=0;i<pSys->sizeof_dev_tested;i++ ) {
    Device_t* pDev      = &pSys->devs[i];

    QString right_str   = QString("正常");
    QString right_color = "#2b2a4c";
    if ( pDev->unit_exception> 0 ) {
      right_color = "#F33C70";
      right_str   = QString("异常");
    } else {
      if ( pDev->booting_stage == 0 ) {
        right_color = "#6699FF";
        right_str   = QString("离线");
      }
    }
    m_model->refresh( 2+i,
                      QString("设备[%1]").arg(i+1),
                      right_str, 
                      right_color);
  }
  /// 
  int temp               = 0;
  int tx_rx              = 0;
  static int can0_tx_pre = 0;
  static int can0_rx_pre = 0;
  get_can_tx(0,&temp);
  Uds_tester::m_sys.status.can_tx_fps_0 = temp - can0_tx_pre;
  tx_rx                     = Uds_tester::m_sys.status.can_tx_fps_0;
  can0_tx_pre               = temp;

  temp = 0;
  get_can_rx(0,&temp);
  Uds_tester::m_sys.status.can_rx_fps_0  = temp - can0_rx_pre;
  tx_rx                     += Uds_tester::m_sys.status.can_rx_fps_0;
  can0_rx_pre                = temp;
  Uds_tester::m_sys.status.can_load_0    = tx_rx * 0.0512;

  tx_rx = 0;
  temp = 0;
  static int can1_tx_pre = 0;
  static int can1_rx_pre = 0;
  get_can_tx(1,&temp);
  Uds_tester::m_sys.status.can_tx_fps_1 = temp - can1_tx_pre;
  tx_rx                     = Uds_tester::m_sys.status.can_tx_fps_1;
  can1_tx_pre               = temp;

  temp = 0;
  get_can_rx(1,&temp);
  Uds_tester::m_sys.status.can_rx_fps_1  = temp - can1_rx_pre;
  tx_rx                     += Uds_tester::m_sys.status.can_rx_fps_1;
  can1_rx_pre                = temp;
  Uds_tester::m_sys.status.can_load_1    = tx_rx * 0.0512;

  return ;
}

// QML层选择左侧列表,读写当前选中项索引号
int Main_data::
get_menu_index(void) const
{ 
  return m_menu_index-2;
}

void Main_data::
set_menu_index(const int _index) 
{
  m_model->refresh_highlight( m_menu_index,"#2b2a4c");
  m_menu_index=_index;
  m_model->refresh_highlight( _index,"#ea906c");
}

// QML 层控制测试过程
void Main_data::
run_pause_clicked(void)
{
  Uds_tester* test = Uds_tester::get_instance();
  Test_sys_t* pSys = &Uds_tester::m_sys;

  if ( m_run == 1) {
    Uds_tester::get_instance()->set_running( 0 );
    m_run = 0;
    m_model->refresh(0 ,"暂停");
    //emit run_tick( 1 );
    emit run_tick( 1,pSys->status.acc_status,pSys->status.bat_status );
  } else {
    Uds_tester::get_instance()->set_running( 1 );
    m_run = 1;
    m_model->refresh(0 ,"测试中");
    emit run_tick( 0,pSys->status.acc_status,pSys->status.bat_status );
  }
  return ;
}
void Main_data::
clean_clicked(void)
{
  //Test_sys_t* pSys = &Uds_tester::m_sys;
  //for( int i=0;i<pSys->sizeof_dev_tested;i++ ) {
  //  Device_t* pDev = &pSys->devs[i];
  //  pDev->req_pending_cnt = 0;
  //  for ( int j=0;j<_SIZE_OF_OP;j++ ) {
  //    //UDS_op_t* op = &pDev->op_list[j];
  //    //op->result = 0;
  //  }
  //}
  return ;
}

void Main_data::
dev_record_clicked( int _dev_index )
{
  return ;
}
void Main_data::
devs_record_clicked(void)
{
  return ;
}

void Main_data::
manual_ctrl_click(int _val)
{
  Uds_tester* test = Uds_tester::get_instance();
  Test_sys_t* pSys = &Uds_tester::m_sys;

  m_run = 0;
  m_model->refresh(0 ,"手动控制");
  Uds_tester::get_instance()->set_running( 0 );
  if ( _val > 0 ) {
    Uds_tester::get_instance()->set_acc_bat( 1 );
    emit run_tick( 0,pSys->status.acc_status,pSys->status.bat_status );
  } else {
    Uds_tester::get_instance()->set_acc_bat( 0 );
    emit run_tick( 0,pSys->status.acc_status,pSys->status.bat_status );
  }
  return ;
}

int read_data( const char* _file_name,
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

int write_data( const char* _file_name ,
                      char *_buf,
                const int _size )
{
  int fd = open( _file_name,
                 O_RDWR|O_CREAT|O_TRUNC,
                 S_IRUSR | S_IWUSR );
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

// 读取测试控制器收发CAN计数
int get_can_rx( const int _port,int* _val )
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  char    buffer[512]    = {'\0'};
  char    file_name[512] ;
  FILE*   fd             = NULL;

  sprintf( file_name,
           "/sys/class/net/can%i/statistics/rx_packets",
           _port);
  fd = fopen( file_name ,"r" );

  if ( fd != NULL ) {
    fgets(buffer, ARRAY_SIZE(buffer), fd);
    sscanf( buffer,"%u", _val);
    fclose(fd);
  } else {
    return -1;
  }
  return 0;
  #else
  *_val = 0;
  return 0;
  #endif
#endif
}

int get_can_tx( const int _port,int* _val )
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  char    buffer[512]    = {'\0'};
  char    file_name[512] ;
  FILE*   fd             = NULL;

  sprintf( file_name,
           "/sys/class/net/can%i/statistics/tx_packets",
           _port);
  fd = fopen( file_name ,"r" );

  if ( fd != NULL ) {
    fgets(buffer, ARRAY_SIZE(buffer), fd);
    sscanf( buffer,"%u", _val);
    fclose(fd);
  } else {
    return -1;
  }
  return 0;
  #else
  *_val = 0;
  return 0;
  #endif
#endif
}
/// end of code ///
