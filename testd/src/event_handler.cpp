#include "event_handler.h"

#include <stdlib.h>
#include <stdio.h>
#include "service/udsManager.h"

#include "support/debug_out.h"
#include "support/c_debug_out.h"

#include "support/utils.h"
#include "support/sys_info.h"
#include "main.h"

#include <mutex>

//using namespace std;

Event_handler::Event_handler(){}
Event_handler::~Event_handler(){}
//////////////////////////////////////////
// 以下回调函数处于多线程环境,如果在多个函数
// 中存在对共享数据的访问,则必须采取适当保护, 
// 如下:
/*
 *
{ // 准备进入临界区域,访问共享对象,首先获取互斥锁
  std::lock_guard<std::mutex> lock( Spy_srv::uds_dev_mtx );
  // 获得互斥锁, 进入临界区域操作
} // 离开临界区域,自动解除锁定
*  
*/
//
///////////////////////////////////////////
///

std::mutex mutext_id_0cef;

// 在 Net_can 层已经拦截了目的地址不是本机的ID,
// 所以能够进入这里的数据一定是满足:
// 源地址=测试控制器00,且目标地址=本机地址,如:01
//
// 注意:
// 可能存在以下情况:由于Socket层的原因,可能在短时间
// 内挤压多个符合条件的消息集中释放到用户层,可能存在
// 消息"踩踏"情况,进而破坏数据
int event_0CEF_UDS( char* _buf,int _dlc)
{
  UNUSED(_dlc);
  // 输入参数是通过指针传入,属于 Net_can::callback_receive() [Thread]
  // 内部的 struct can_frame.data[],
  // 而运行至此,已经处于线程池中的某线程内部,所以采取以下两个措施:
  // 1. 添加mutex保护
  // 2. 进入工作线程后,复制后再后续处理

  // 以下代码段受mutex保护,花括号不能删除 !
  {
    // 实际上"阻塞"于此等待互斥信号释放
    std::lock_guard<std::mutex> lock( mutext_id_0cef );
    uint8_t msg_data[8];
    memcpy( msg_data,_buf,_dlc );
    UdsManager::getInstance()->messageHandler(msg_data,_dlc);
  // 离开花括号,也就是离开mutex的保护范围
  }
  return 0;
}

int event_0CEE_UDS( char*_buf,int _dlc)
{
  UNUSED(_buf);
  UNUSED(_dlc);
  return 0;
}

int event_0CEA( char* _buf,int _dlc) 
{
  UNUSED(_buf);
  UNUSED(_dlc);
  return 0;
}

int event_0CEB( char* _buf,int _dlc) 
{
  UNUSED(_buf);
  UNUSED(_dlc);
  return 0;
}
///////////////////////////////////////////
/// 完成:CAN消息回调函数定义

/// end of code ///
