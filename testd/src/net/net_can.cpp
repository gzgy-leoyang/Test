#include "net/net_can.h"
#include <thread>
#include <iostream>
#include <unistd.h>

//#include "support/debug_out.h"
#include "support/threadpool.h"
#include <sys/syscall.h>
#include "main.h"

struct can_frame                   ttx_frame;

void Net_can::
callback_receive(void)
{
  pthread_setname_np(pthread_self(),
                     "callback_receive");
  struct can_frame frame;
  while ( true ) {
    int ret = recv(m_fd,&frame,
                   sizeof(struct can_frame),0);
    if(ret > 0){
      char src_addr = (frame.can_id & 0x000000FF) ;
      char des_addr = (frame.can_id & 0x0000FF00) >> 8 ;
      if ( (src_addr == m_tester_address) && 
           (des_addr == m_this_address  )) {
        // 来自Tester,且目的地址为当前设备,执行接收处理
        callback_t cb = m_ID_callback [ frame.can_id ] ;
        if ( cb != nullptr ) {
          if ( m_pool != nullptr ) {
            m_pool->submit(cb,
                           reinterpret_cast<char*>(frame.data),
                           frame.can_dlc);
          } else {
            cb(reinterpret_cast<char*>(frame.data),frame.can_dlc);
          }
        }
      }
    }
  }
  return ;
}

Net_can::
Net_can( const char _addr,
         const char _ta,
         const char* _can,
         const int   _ts):
  Abstract_net()
{
  m_can_x          = 0;
  m_id_filted      = 0;
  m_this_address   = _addr;
  m_tester_address = _ta;
  memset( reinterpret_cast<char*>(&m_ifreq),0,sizeof(struct ifreq));
  memset( reinterpret_cast<char*>(&m_canAddr),0,sizeof(struct sockaddr_can));

  if ( _can == nullptr ) {
    LOGE<<" Port name is nullptr,return..";
    return ;
  }

  int fd = socket( PF_CAN, SOCK_RAW, CAN_RAW);
  if( fd < 0 ){
    LOGE<<" Failed to create socketCAN,return..";
    return;
  }

  m_fd = fd;
  strcpy( m_ifname ,_can);
  strcpy( m_ifreq.ifr_name ,_can);
  ioctl(m_fd, SIOCGIFINDEX, &m_ifreq);
  m_canAddr.can_family  = PF_CAN;
  m_canAddr.can_ifindex = m_ifreq.ifr_ifindex;

  if( bind ( m_fd ,
             reinterpret_cast<struct sockaddr*>(&m_canAddr),
             sizeof(m_canAddr) ) == -1 ){
    LOGE<<" Failed to bind socketCAN";
    close( m_fd );
  }
  LOGV<<" Port:"<<_can<<" SocketCAN Device: "<<m_fd ;

  if ( _ts > 0 ) {
    m_pool = new ThreadPool ( _ts );
    m_pool->init();
    LOGV <<" Thread pool was created ";
  }
  m_p1 = std::thread(&Net_can::callback_receive,this);
  m_p1.detach();
  LOGV <<" Receive thread was created ";
}

Net_can::
Net_can( const char _addr,const char _ta, const char* _can):
  Abstract_net()
{
  m_can_x          = 0;
  m_id_filted      = 0;
  m_this_address   = _addr;
  m_tester_address = _ta;
  memset( reinterpret_cast<char*>(&m_ifreq),0,sizeof(struct ifreq));
  memset( reinterpret_cast<char*>(&m_canAddr),0,sizeof(struct sockaddr_can));

  if ( _can == nullptr ) {
    LOGE<<" Port name is nullptr,return..";
    return ;
  }

  int fd = socket( PF_CAN, SOCK_RAW, CAN_RAW);
  if( fd < 0 ){
    LOGE<<" Failed to create socketCAN,return..";
    return;
  }

  m_fd = fd;
  strcpy( m_ifname ,_can);
  strcpy( m_ifreq.ifr_name ,_can);
  ioctl(m_fd, SIOCGIFINDEX, &m_ifreq);
  m_canAddr.can_family  = PF_CAN;
  m_canAddr.can_ifindex = m_ifreq.ifr_ifindex;

  if( bind ( m_fd ,
             reinterpret_cast<struct sockaddr*>(&m_canAddr),
             sizeof(m_canAddr) ) == -1 ){
    LOGE<<" Failed to bind socketCAN";
    close( m_fd );
  }
  LOGV<<" Port:"<<_can<<" SocketCAN Device: "<<m_fd ;

  m_pool = new ThreadPool (3);
  m_pool->init();
  LOGV <<" Thread pool was created ";

  m_p1 = std::thread( &Net_can::callback_receive,this);
  m_p1.detach();
  LOGV <<" Receive thread was created ";

}

Net_can::
Net_can( const char* _can) :
  Abstract_net()
{
  m_can_x          = 0;
  m_id_filted      = 0;
  memset( reinterpret_cast<char*>(&m_ifreq),0,sizeof(struct ifreq));
  memset( reinterpret_cast<char*>(&m_canAddr),0,sizeof(struct sockaddr_can));

  if ( _can == nullptr ) {
    LOGE<<" Port name is nullptr,return..";
    return ;
  }

  int fd = socket( PF_CAN, SOCK_RAW, CAN_RAW);
  if( fd < 0 ){
    LOGE<<" Failed to create socketCAN,return..";
    return;
  }

  m_fd = fd;
  strcpy( m_ifname ,_can);
  strcpy( m_ifreq.ifr_name ,_can);
  ioctl(m_fd, SIOCGIFINDEX, &m_ifreq);
  m_canAddr.can_family  = PF_CAN;
  m_canAddr.can_ifindex = m_ifreq.ifr_ifindex;

  if( bind ( m_fd ,
             reinterpret_cast<struct sockaddr*>(&m_canAddr),
             sizeof(m_canAddr) ) == -1 ){
    LOGE<<" Failed to bind socketCAN";
    close( m_fd );
  }
  LOGV<<" Port:"<<_can<<" SocketCAN Device: "<<m_fd ;

  m_pool = new ThreadPool (3);
  m_pool->init();
  LOGV <<" Thread pool was created ";

  m_p1 = std::thread( &Net_can::callback_receive,this);
  m_p1.detach();
  LOGV <<" Receive thread was created ";
}

int Net_can::
register_callback_for_ID( int _id, callback_t _cb )
{
  if ( _cb == nullptr ) {
    LOGE<<" The pointer of Callback is nullptr";
    return -1;
  }
  m_ID_callback.insert(std::pair<int, callback_t>((_id|0x80000000),_cb) );
  return 0;
}

int Net_can::
transmit_message( const int& _info,
                  char* _buf,
                  const int& _size )
{
  if ( _size> 8 ){
    printf( "Net_can::send (uint8_t _dlc) > 8\n");
    return -1;
  }

  if ( _buf== (void*)NULL ){
    printf( "Net_can::send (uint8_t* _dat) == NULL\n" );
    return -1;
  }

  // 如果大于11位的位置全为0,则按照标准帧发送,否则按照扩展帧发送
  if ( ( _info & (~CAN_SFF_MASK)) == 0){
    ttx_frame.can_id = _info & (~CAN_EFF_FLAG) ;
  } else {
    ttx_frame.can_id = _info | CAN_EFF_FLAG ;
  }
  ttx_frame.can_dlc = _size;
  memcpy( ttx_frame.data, _buf, _size );

  if ( m_fd == 0 ){
    printf( "Net_can::send m_s_fd == 0\n" );
    return -1;
  }

  int ret = write( m_fd,&ttx_frame,sizeof(struct can_frame) );
  if ( ret <= 0 ){
    printf( " Failed to transmit frame(%08x) ret=%i fd=%i\n ",
            _info,ret, m_fd );
  }

#if _LONG_DEBUG_INFO
  printf("[CAN] Tx %08x {", ttx_frame.can_id );
  for (int i=0;i<ttx_frame.can_dlc;i++ ){
    printf("%02x ",*(ttx_frame.data+i) );
  }
  printf("}\n");
#endif

  return 0;
}
/// end of code ///
