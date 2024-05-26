#include "page_device_data.h"
#include <map>
#include <iterator>
#include <QDebug>

#include "misc/uds_tester.h"
//#include "misc/tester/tester.h"
#include "misc/main_data.h"
#include "main.h"

using namespace std;

Page_device_data::
Page_device_data(QObject *parent)
    : QObject{parent}
{
  m_model = new Data_model(this);

  // 添加右侧列表设备项
  //Uds_tester* test = Uds_tester::get_instance();
  Test_sys_t* pSys = &Uds_tester::m_sys;
  Device_t dev = pSys->devs[0];
  for ( int j=0;j<_SIZE_OF_OP;j++ ) {
    UDS_op_t* op = &Uds_tester::dev_op[j];
    //UDS_op_t* op = &dev.op_list[j];
    m_model->pushData(new Data_element(QString(op->desp),
                                       QString("#2b2a4c"),
                                       QString("--"),
                                       QString("#2b2a4c"),
                                       this));
  }

  // 1s 更新一次右侧测试参数列表
  m_tim = new QTimer(this);
  connect(m_tim,SIGNAL(timeout()),this,SLOT(timeout_slot()));
  m_tim->start(100);
}

void Page_device_data::
timeout_slot()
{
  unsigned long flag = 0x0000000000000001;
  // 获取当前指向的设备索引,右侧列表更新
  // 对应设备的参数
  int index = main_data->get_menu_index();
  if ( index >= 0 ) {
    // 更新指定设备的测试参数列表
    //Uds_tester* test = Uds_tester::get_instance();
    Test_sys_t* pSys = &Uds_tester::m_sys;
    Device_t dev = pSys->devs[index];

    for ( int j=0;j<_SIZE_OF_OP;j++ ) {
      UDS_op_t* op = &Uds_tester::dev_op[j];
      QString left_str = QString(op->desp);

      int did = op->did;
      // 通过DID,计算参数矩阵中的坐标,取值
      uint8_t unit_index  = ((did-0x8000) >> 8) & 0x1F;
      uint8_t param_index = (did & 0x00FF);
      Testing_param_t* pParam = &dev.val_matrix[unit_index][param_index]; 

      QString right_str;
      // 当被请求数据是否定应答时,数据区在发起请求时被设置为fffffff
      if ( pParam->val != 0xFFFFFFFF ) {
        if ( static_cast<int>(op->rate) != 1 ) {
          float ff = pParam->val* op->rate;
          right_str = QString("%1").
                        arg(ff,0,'g',3);
        } else {
          right_str = QString("%1").arg( pParam->val );
        }
      } else {
        right_str = QString("Unvalid");
      }
      QString color = QString("#2b2a4c");
      //异常标记,高亮显示,需要对所有参数分单元之后,才能按定位到异常参数高亮
      if ( pParam->exception_flag != 0 ) {
        color = QString("#b31312");
        right_str += QString("(%1)").arg( pParam->exception_flag );
      }
      m_model->refresh(j,left_str,color,right_str,color);
    }
  }
  return ;
}
/// end of code ///
