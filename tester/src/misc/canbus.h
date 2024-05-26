#ifndef CANBUS_H
#define CANBUS_H

#include <QObject>
#include <QThread>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/time.h>
#include <time.h>
#include <libgen.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>
#include <sys/prctl.h>
#include <sys/syscall.h>   /* Definition of SYS_* constants */

#include <queue>

typedef void (*callback_ptr)(struct can_frame* _frame );

class Canbus :public QObject
{
  Q_OBJECT
public:
  static Canbus* get_instance(void) { return m_net; }

  void set_receive_callback(callback_ptr _ptr){
    Canbus::m_receive_callback = _ptr;
  }
  int send_message( const char* _buf,const int _size );

  int set_tx_load( const int _ch, const int _precent );
  int enable_tx( const int _ch, const int _enable);

  static int              m_fd_0;
  static int              m_fd_1;

  static callback_ptr     m_receive_callback;
  static int m_rx_cnt;
  static int m_tx_cnt;
  static int m_enable_tx_can0;
  static int m_enable_tx_can1;
  static std::queue<struct can_frame> m_msgs;

private:
  static Canbus* m_net;
  explicit Canbus(QObject* parent = 0);
  ~Canbus(void);

  pthread_t m_receive_thread;
  pthread_t m_transmit_thread;

  pthread_t m_pressure_transmit_thread;
};
#endif // CAN_NET_H
/// end of code ///
