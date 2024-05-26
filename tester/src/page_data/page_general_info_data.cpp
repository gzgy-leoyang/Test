#include "page_general_info_data.h"
#include "misc/uds_tester.h"
//#include "misc/tester/tester.h"
#include "main.h"

#include <QDateTime>
//#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
///////////////////////////////////////////////
Page_general_info_data::
Page_general_info_data(QObject *parent)
    : QObject{parent}
{
  // 在右侧列表添加测试情况统计
  m_model = new Data_model(this);
  for( int i=0;i<sizeof(m_menu)/sizeof(Data_item_t);i++ ){
      Data_item_t* mi = &m_menu[i];
      m_model->pushData(new Data_element(mi->left_str,
                                         mi->left_color,
                                         mi->right_str,
                                         mi->right_color,
                                         this));
  }

  ///更新本次启动时间
  QDateTime dt = QDateTime::currentDateTime();
  QString date_time = dt.toString("yyyy-MM-dd hh:mm:ss");
  m_model->refresh(1,date_time );

  m_tim = new QTimer(this);
  connect(m_tim,SIGNAL(timeout()),this,SLOT(timeout_slot()));
  m_tim->start(500);
}

// 每秒更新测试统计情况
void Page_general_info_data::
timeout_slot()
{
  Test_sys_t* sys = &Uds_tester::m_sys ;
  int hour = 0;
  int min  = 0;
  int sec  = 0;
  int temp = 0;


  m_model->refresh(0, QString::fromStdString(sys->config_file_name));
  m_model->refresh(2, QString("%1 次").arg(sys->status.preoid_cycle));
  m_model->refresh(3, QString("%1 次").arg(sys->total_cycle));

  hour = sys->status.preoid_second/ 3600;
  temp = sys->status.preoid_second% 3600;
  min  = temp / 60;
  sec  = temp % 60;
  m_model->refresh(4, QString("%1小时%2分%3秒").
                      arg(hour,2,10,QChar('0')).
                      arg(min ,2,10,QChar('0')).
                      arg(sec ,2,10,QChar('0')));
  hour = sys->total_second / 3600;
  temp = sys->total_second % 3600;
  min  = temp / 60;
  sec  = temp % 60;
  m_model->refresh(5, QString("%1小时%2分%3秒").
                      arg(hour,2,10,QChar('0')).
                      arg(min ,2,10,QChar('0')).
                      arg(sec ,2,10,QChar('0')));

  m_model->refresh(6, QString("%1 fps").
                        arg(sys->status.can_tx_fps_0));
  m_model->refresh(7, QString("%1 fps").
                        arg(sys->status.can_rx_fps_0));
  m_model->refresh(8, QString("%1 %").
                        arg(sys->status.can_load_0,0,'f',2));

  m_model->refresh(9, QString("%1 fps").
                        arg(sys->status.can_tx_fps_1));
  m_model->refresh(10, QString("%1 fps").
                        arg(sys->status.can_rx_fps_1));
  m_model->refresh(11, QString("%1 %").
                        arg(sys->status.can_load_1,0,'f',2));

  Uds_tester* test = Uds_tester::get_instance();
  // ACC状态
  m_model->refresh(12, test->get_acc_status());
  m_model->refresh(13, test->get_bat_status());
  m_model->refresh(14, test->get_rand_sec());

  return ;
}
/// end of code ///
