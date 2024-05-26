#ifndef KEMP_ADAPTER_H
#define KEMP_ADAPTER_H

#include "adapter.h"
#include <thread>

using namespace std;

class Kemp_adapter : public Adapter 
{
public:
  Kemp_adapter();
  ~Kemp_adapter();

  void key_monitor(void);
  void touch_monitor(void);

  static int getKeyF1PressCounter( char* _buf,int _size ) ;
  static int getKeyF1ReleaseCounter( char* _buf,int _size ) ;
  static int m_F1_pressed;
  static int m_F1_released;

  static int getBtnTouchPressCounter( char* _buf,int _size ) ;
  static int getBtnTouchReleaseCounter( char* _buf,int _size ) ;
  static int getBtnTouchX( char* _buf,int _size ) ;
  static int getBtnTouchY( char* _buf,int _size ) ;

  static int m_btn_touch_pressed;
  static int m_btn_touch_released;
  static int m_btn_touch_y;
  static int m_btn_touch_x;

  static int btnTouchPressCounter_rw( char* _buf,int _size ) ;
  static int btnTouchReleaseCounter_rw( char* _buf,int _size );
private:
  thread m_thread_key;
};

#endif
