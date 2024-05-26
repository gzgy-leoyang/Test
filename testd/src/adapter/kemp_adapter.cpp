#include "adapter/kemp_adapter.h"
#include "support/debug_out.h"
#include "support/sys_info.h"
#include "service/udsManager.h"
#include "main.h"

#include <iostream>
#include <fstream>

#include <sys/prctl.h>

using std::ifstream;
using std::ios;

int Kemp_adapter::m_F1_pressed         = 0;
int Kemp_adapter::m_F1_released        = 0;
int Kemp_adapter::m_btn_touch_y        = 0;
int Kemp_adapter::m_btn_touch_x        = 0;
int Kemp_adapter::m_btn_touch_pressed  = 0;
int Kemp_adapter::m_btn_touch_released = 0;

Kemp_adapter::
Kemp_adapter():Adapter()
{

  UdsManager* um = UdsManager::getInstance();
  UDS_operator_t op;
  op.safety=0;
  op.session=0;
  op.callback=getKeyF1PressCounter;
  um->registeGeneralOperatorWithDid(0x8601,op);

  op.callback=getKeyF1ReleaseCounter;
  um->registeGeneralOperatorWithDid(0x8602,op);

  op.callback=btnTouchPressCounter_rw;
  um->registeGeneralOperatorWithDid(0x8603,op);

  op.callback=btnTouchReleaseCounter_rw;
  um->registeGeneralOperatorWithDid(0x8604,op);

  op.callback=getBtnTouchX;
  um->registeGeneralOperatorWithDid(0x8605,op);

  op.callback=getBtnTouchY;
  um->registeGeneralOperatorWithDid(0x8606,op);

  std::thread thread_key(&Kemp_adapter::key_monitor,this);
  thread_key.detach();

  std::thread thread_touch(&Kemp_adapter::touch_monitor,this);
  thread_touch.detach();
}

int Kemp_adapter::
btnTouchPressCounter_rw( char* _buf,int _size )
{
  int sid = *(_buf+1);
  int br = 0;
  if ( sid == 0x22 ) {
    LOGI<<" Btn touch passed:"<<m_btn_touch_pressed;
    std::memset((_buf+4),0xAA,4);
    *(_buf+4) = ( m_btn_touch_pressed & 0xFF000000) >> 24;
    *(_buf+5) = ( m_btn_touch_pressed & 0x00FF0000) >> 16;
    *(_buf+6) = ( m_btn_touch_pressed & 0x0000FF00) >> 8;
    *(_buf+7) = ( m_btn_touch_pressed & 0x000000FF);
    return 4;
  } else if ( sid == 0x2E ) {
    int temp = *(_buf+4);
    temp     = ( temp << 8 ) | *(_buf+5);
    temp     = ( temp << 8 ) | *(_buf+6);
    temp     = ( temp << 8 ) | *(_buf+7);
    LOGI<<" Set Touch Pressed & Released counter:"<<temp;
    m_btn_touch_pressed = temp;
    m_btn_touch_released= temp;
    std::memset((_buf+4),0xAA,4);
    return 0;
  } else {
    // unknow SID
  }
  return 0;
}

int Kemp_adapter::
btnTouchReleaseCounter_rw( char* _buf,int _size )
{
  int sid = *(_buf+1);
  int br = 0;
  if ( sid == 0x22 ) {
    LOGI<<" Btn touch release:"<<m_btn_touch_released;
    std::memset((_buf+4),0xAA,4);
    *(_buf+4) = ( m_btn_touch_released & 0xFF000000) >> 24;
    *(_buf+5) = ( m_btn_touch_released & 0x00FF0000) >> 16;
    *(_buf+6) = ( m_btn_touch_released & 0x0000FF00) >> 8;
    *(_buf+7) = ( m_btn_touch_released & 0x000000FF);
    return 4;
  } else if ( sid == 0x2E ) {
    int temp = *(_buf+4);
    temp     = ( temp << 8 ) | *(_buf+5);
    temp     = ( temp << 8 ) | *(_buf+6);
    temp     = ( temp << 8 ) | *(_buf+7);
    LOGI<<" Set Touch Pressed & Released counter:"<<temp;
    m_btn_touch_pressed = temp;
    m_btn_touch_released= temp;
    std::memset((_buf+4),0xAA,4);
    return 0;
  } else {
    // unknow SID
  }
  return 0;
}

int Kemp_adapter::
getKeyF1PressCounter( char* _buf,int _size )
{
  UNUSED(_size);

  LOGI<<" F1 Pressed:"<<m_F1_pressed;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( m_F1_pressed & 0xFF000000) >> 24;
  *(_buf+5) = ( m_F1_pressed & 0x00FF0000) >> 16;
  *(_buf+6) = ( m_F1_pressed & 0x0000FF00) >> 8;
  *(_buf+7) = ( m_F1_pressed & 0x000000FF);
  return 4;
}

int Kemp_adapter::
getKeyF1ReleaseCounter( char* _buf,int _size )
{
  UNUSED(_size);

  LOGI<<" F1 Release:"<<m_F1_released;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( m_F1_released & 0xFF000000) >> 24;
  *(_buf+5) = ( m_F1_released & 0x00FF0000) >> 16;
  *(_buf+6) = ( m_F1_released & 0x0000FF00) >> 8;
  *(_buf+7) = ( m_F1_released & 0x000000FF);
  return 4;
}

int Kemp_adapter::
getBtnTouchPressCounter( char* _buf,int _size )
{
  UNUSED(_size);
  LOGI<<" Btn touch passed:"<<m_btn_touch_pressed;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( m_btn_touch_pressed & 0xFF000000) >> 24;
  *(_buf+5) = ( m_btn_touch_pressed & 0x00FF0000) >> 16;
  *(_buf+6) = ( m_btn_touch_pressed & 0x0000FF00) >> 8;
  *(_buf+7) = ( m_btn_touch_pressed & 0x000000FF);
  return 4;
}

int Kemp_adapter::
getBtnTouchReleaseCounter( char* _buf,int _size )
{
  UNUSED(_size);
  LOGI<<" Btn touch release:"<<m_btn_touch_released;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( m_btn_touch_released & 0xFF000000) >> 24;
  *(_buf+5) = ( m_btn_touch_released & 0x00FF0000) >> 16;
  *(_buf+6) = ( m_btn_touch_released & 0x0000FF00) >> 8;
  *(_buf+7) = ( m_btn_touch_released & 0x000000FF);
  return 4;
}

int Kemp_adapter::
getBtnTouchX( char* _buf,int _size )
{
  UNUSED(_size);
  LOGI<<" Btn touch X:"<<m_btn_touch_x;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( m_btn_touch_x & 0x0000FF00) >> 8;
  *(_buf+5) = ( m_btn_touch_x & 0x000000FF);
  return 2;
}

int Kemp_adapter::
getBtnTouchY( char* _buf,int _size )
{
  UNUSED(_size);
  LOGI<<" Btn touch Y:"<<m_btn_touch_y;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = ( m_btn_touch_y & 0x0000FF00) >> 8;
  *(_buf+5) = ( m_btn_touch_y & 0x000000FF);
  return 2;
}
void Kemp_adapter::
touch_monitor(void)
{
  char event_name[32];
  memset(event_name,'\0',32);
  if (sysInfo_getInputDeviceEventByName("ili2511",event_name)<0) {
    LOGW<<" Did NOT found input device:ili2511";
    return ;
  }

  char device_name[128];
  memset(device_name,'\0',128);
  sprintf(device_name,"/dev/input/%s",event_name);
  int ts_fd=open(device_name,O_RDONLY);
  if(ts_fd<=0){
    LOGW<<"Input device:ili2511:"<<device_name<<" is NOT exist";
    return ;
  }
  LOGI<<"TouchScreen<-->"<<device_name;

  prctl(PR_SET_NAME,"Touch monitor");
  struct input_event t;
  while( true ){
    if(read(ts_fd,&t,sizeof(struct input_event))>0) {
      if(t.type==1) {
        if ( t.code == BTN_TOUCH ) {
          if ( t.value == 0 )  {
            m_btn_touch_released++;
          } else if ( t.value == 1 ) {
            m_btn_touch_pressed++;
          } else {
          }
        } else {
          // 其他按键
        }
      } else if( t.type == 3 ) {
        if ( t.code == ABS_Y) {
          m_btn_touch_y = t.value;
        } else if ( t.code == ABS_X) {
          m_btn_touch_x = t.value;
        } else {
          // 其他按键
        }
      } else {
        // 其他动作,EV_ABS等
      }
      //LOGV<<" EV_KEY  P="<<m_btn_touch_pressed<<" R="<<m_btn_touch_released;
      //LOGV<<" EV_ABS  X="<<m_btn_touch_x<<" Y="<<m_btn_touch_y;
      memset(&t,0,sizeof(struct input_event));
    }
  }
  close(ts_fd);
  return ;
}



void Kemp_adapter::
key_monitor(void)
{

  char event_name[32];
  memset(event_name,'\0',32);
  if (sysInfo_getInputDeviceEventByName("Kemp",event_name)<0) {
    LOGW<<" Did NOT found input device:Kemp";
    return ;
  }

  char device_name[128];
  memset(device_name,'\0',128);
  sprintf(device_name,"/dev/input/%s",event_name);
  int keys_fd=open(device_name,O_RDONLY);
  if(keys_fd<=0){
    LOGW<<"Input device:Kemp "<<device_name<<" is NOT exist";
    return ;
  }
  LOGI<<"Kemp <-->"<<device_name;

  prctl(PR_SET_NAME,"Key monitor");
  struct input_event t;
  while( true ){
    if(read(keys_fd,&t,sizeof(struct input_event))>0) {
      if(t.type==1) {
        if ( t.code == KEY_F1 ) {
          if ( t.value == 0 )  {
            m_F1_released++;
          } else if ( t.value == 1 ) {
            m_F1_pressed++;
          } else {
          }
        } else {
          // 其他按键
        }
      } else {
        // 其他动作,EV_ABS等
      }
      //LOGV<<" EV_KEY F1 P="<<m_F1_pressed<<" R="<<m_F1_released;
      memset(&t,0,sizeof(struct input_event));
    }
  }
  close(keys_fd);
  return ;
}
/*
struct input_event {
  struct timeval time;
  __u16 type; //类型:EV_SYN,EV_KEY,EV_REL,EV_ABS,EV_MSC,EV_LED
  __u16 code; //键值
  __s32 value;//键状态
};
 * */
/// end of code ///
