#include "canbus.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

#include <stdio.h>
#include <errno.h>

/*
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  #else
  #endif
#endif
*/

#define _CM_CTS         (17) //控制字16的多包，接收开始应答17
#define _CM_END_OF_MSG  (19) //控制字16的多包，接收完成应答19

Canbus*  Canbus::m_net = new Canbus();

int Canbus::m_fd_0           = 0;
int Canbus::m_fd_1           = 0;
int Canbus::m_rx_cnt         = 0;
int Canbus::m_tx_cnt         = 0;
int Canbus::m_enable_tx_can0 = 0;
int Canbus::m_enable_tx_can1 = 0;

int delay_reload_can0        = 10;
int delay_reload_can1        = 10;

callback_ptr                 Canbus::m_receive_callback = nullptr;
// 接收来自uds-test 模块的测试消息
std::queue<struct can_frame> Canbus::m_msgs;

void* net_receiver(void* arg)
{
  (void)arg;
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  prctl(PR_SET_NAME,"UDSTesterRx");
  if ( Canbus::m_fd_0 <= 0){
    return NULL;
  }

  struct can_frame frame;
  while(1){
    // 1. 发出UDS请求
    // 2. 阻塞,等待回复, 注意排除被其他ID触发,需要滤波处理
    if ( read(Canbus::m_fd_0,
              &frame,
              sizeof(struct can_frame)) > 0 ){
      int id = frame.can_id & ~0x80000000;
      if ((id == 0x00AA0301)||((id & 0x0000FF00)== 0x00)){
        // 确实是发送给 Tester 的消息
        char src_addr = id & 0x000000FF;
        if ( Canbus::m_receive_callback != nullptr ) {
          Canbus::m_receive_callback( &frame );
        }
      }
    }
  }
  #else
  #endif
#endif
  return NULL;
}

void* net_transmiter(void* arg)
{
  (void)arg;
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  int retry = 0;
  int ret   = 0;
  prctl(PR_SET_NAME,"UDSTesterTx");
  while ( 1 ) {
    if (Canbus::m_msgs.empty()){
      usleep(10000);
    } else {
      struct can_frame frame = Canbus::m_msgs.front();
      do {
        ret =  write( Canbus::m_fd_0,
                      (const void*)&frame,
                      sizeof(struct can_frame));
        if ( ret == sizeof( struct can_frame )) {
        } else {
          usleep(100);
          retry++;
        }
      } while((ret<0) && (retry<2));
      Canbus::m_msgs.pop();
      ret   = 0;
      retry = 0;
    }
  }
  #else
  #endif
#endif
  return NULL;
}

typedef struct {
  uint32_t id;
  uint8_t dat[8];
} Frame_t;


static int load_asc_file( const QString& _asc,
                          Frame_t*       _pList,
                          int            _size )
{
//#if defined(Q_OS_WIN)
//  #elif defined(Q_OS_LINUX)
//  #ifdef __ARM_ARCH
//  #else
//  #endif
//#endif
  int index = 0;
  QFile asc_file( _asc );
  if ( asc_file.exists()) {
    if ( asc_file.open(QIODevice::ReadOnly )) {
      QTextStream in(&asc_file);
      while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.contains("Rx")) {
          QStringList ll = line.split(' ');
          if ( ll.count() >= 6 ) {
            bool ok;
            Frame_t* pf = (Frame_t*)(_pList+index);
            pf->id = 0;
            memset((uint8_t*)pf->dat,0xFF,8);

            QString id_str = ll[2];
            id_str.remove(QChar('x'));
            pf->id = id_str.toInt(&ok,16);

            uint8_t dlc = ll[5].toInt(&ok,10);

            for (uint8_t i=0;i<dlc;i++) {
              uint8_t dd = ll[6+i].toInt(&ok,16);
              if ( !ok )
                dd = 0xFF;
              *(pf->dat+i) = dd;
            }
            index++;
            if ( index >= _size ) {
              goto exit;
            }
          }
        }
      }
    }
  } else {
    goto err;
  }

exit:
  asc_file.close();
  return index;
err:
  return -1;
}

void* net_pressure_transmiter(void* arg)
{
  (void)arg;
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  Frame_t list_can0[10000];
  Frame_t list_can1[10000];
  int sizeof_list_can0 = 0;
  int sizeof_list_can1 = 0;

  int ret = 0;
  struct can_frame frame;
  static int can_0_index = 0;
  static int can_1_index = 0;

  static int can_0_delay = 0;
  static int can_1_delay = 0;

  static int fps_cal_preoid = 0;

  prctl(PR_SET_NAME,"CanPressureTx");

  memset( (char*)&frame.data,0,sizeof(struct can_frame));

  sizeof_list_can0= load_asc_file( "/home/root/can_0.asc",list_can0,10000);
  sizeof_list_can1= load_asc_file( "/home/root/can_1.asc",list_can1,10000);
  if ( (sizeof_list_can0 == 0) || (sizeof_list_can1 == 0) ) {
    return NULL;
  }

  while ( 1 ) {
    if ( Canbus::m_enable_tx_can0 > 0 ) {
      can_0_delay--;
      if ( can_0_delay <= 0 ) {
        can_0_delay = delay_reload_can0;
        can_0_index++;
        if ( can_0_index >= sizeof_list_can0 )
          can_0_index = 0;
        Frame_t* pF  = &list_can0[ can_0_index];
        frame.can_id = pF->id | CAN_EFF_FLAG;
        frame.can_dlc = 8;
        memcpy( frame.data,pF->dat,8 );
        ret =  write( Canbus::m_fd_0,
                      (const void*)&frame,
                      sizeof(struct can_frame));
      }
    }
    usleep(100);

    if ( Canbus::m_enable_tx_can1 > 0 ) {
      can_1_delay--;
      if ( can_1_delay <= 0 ) {
        can_1_delay = delay_reload_can1;

        Frame_t* pF  = &list_can1[ can_1_index];
        can_1_index++;
        if ( can_1_index >= sizeof_list_can1 )
          can_1_index = 0;

        frame.can_id = pF->id | CAN_EFF_FLAG;
        frame.can_dlc = 8;
        memcpy( frame.data,pF->dat,8 );
        ret =  write( Canbus::m_fd_1,
                      (const void*)&frame,
                      sizeof(struct can_frame));
      }
    }
    usleep(100);
  }
  #else
  #endif
#endif
  return NULL;
}


Canbus::Canbus(QObject* parent):
  QObject(parent)
{
  m_rx_cnt     = 0;
  m_tx_cnt     = 0;

#if defined(Q_OS_WIN)

#elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH

  int                     m_id_filted;
  char                    m_ifname[20];
  struct  ifreq           m_ifreq;
  struct  sockaddr_can    m_canAddr;
  m_fd_0 = socket( PF_CAN, SOCK_RAW, CAN_RAW);
  if( m_fd_0 < 0 ){
    return ;
  } else {
    char* buf= "can0";
    strcpy( m_ifname ,buf);
    strcpy( m_ifreq.ifr_name ,buf);
    ioctl ( m_fd_0, SIOCGIFINDEX, &m_ifreq);
    m_canAddr.can_family  = PF_CAN;
    m_canAddr.can_ifindex = m_ifreq.ifr_ifindex;

    if( bind(m_fd_0,
               (struct sockaddr*)&m_canAddr,
               sizeof(m_canAddr)) == -1 ){
        close(m_fd_0);
        printf("Failed to BIND\n");
        return ;
    } else {
      // 添加滤波
      struct can_filter filter_list[4] = {{0,0}};
      filter_list[0].can_id   = 0x0CEF0000 | CAN_EFF_FLAG;
      filter_list[0].can_mask = 0x0FFF0000;
      filter_list[1].can_id   = 0x1CF00000 | CAN_EFF_FLAG;
      filter_list[1].can_mask = 0x1FF00000;
      filter_list[2].can_id   = 0x1CF10000 | CAN_EFF_FLAG;
      filter_list[2].can_mask = 0x1FF10000;
      filter_list[3].can_id   = 0x00AA0301 | CAN_EFF_FLAG;
      filter_list[3].can_mask = 0x00AA0301 ;
      if( setsockopt(m_fd_0,
                     SOL_CAN_RAW,
                     CAN_RAW_FILTER,
                     &filter_list,
                     sizeof( filter_list )) < 0){
        printf("Failed to set filter\n");
      }

      if (pthread_create(&m_receive_thread,
                         NULL,
                         net_receiver,
                         NULL)<0){
        perror(" Failed to create receive thread");
      }

      if (pthread_create(&m_transmit_thread,
                         NULL,
                         net_transmiter,
                         NULL)<0){
        perror(" Failed to create transmit thread");
      }
    }
  }

  // 用于UDS测试
  m_fd_1 = socket( PF_CAN, SOCK_RAW, CAN_RAW);
  if( m_fd_1 < 0 ){
    return ;
  } else {
    char* buf= "can1";
    strcpy( m_ifname ,buf);
    strcpy( m_ifreq.ifr_name ,buf);
    ioctl ( m_fd_1, SIOCGIFINDEX, &m_ifreq);
    m_canAddr.can_family  = PF_CAN;
    m_canAddr.can_ifindex = m_ifreq.ifr_ifindex;

    if( bind(m_fd_1,
               (struct sockaddr*)&m_canAddr,
               sizeof(m_canAddr)) == -1 ){
        close(m_fd_1);
        printf("Failed to BIND\n");
        return ;
    } else {
      // 添加滤波
      struct can_filter filter_list[4] = {{0,0}};
      filter_list[0].can_id   = 0x0CEF0000 | CAN_EFF_FLAG;
      filter_list[0].can_mask = 0x0FFF0000;
      filter_list[1].can_id   = 0x1CF00000 | CAN_EFF_FLAG;
      filter_list[1].can_mask = 0x1FF00000;
      filter_list[2].can_id   = 0x1CF10000 | CAN_EFF_FLAG;
      filter_list[2].can_mask = 0x1FF10000;
      filter_list[3].can_id   = 0x00AA0301 | CAN_EFF_FLAG;
      filter_list[3].can_mask = 0x00AA0301 ;
      if( setsockopt(m_fd_1,
                     SOL_CAN_RAW,
                     CAN_RAW_FILTER,
                     &filter_list,
                     sizeof( filter_list )) < 0){
        printf("Failed to set filter\n");
      }
    }
  }

  Canbus::m_enable_tx_can0=0;
  Canbus::m_enable_tx_can1=0;
  if (pthread_create(&m_pressure_transmit_thread,
                     NULL,
                     net_pressure_transmiter,
                     NULL)<0){
    perror(" Failed to create receive thread");
  }
  #else
  // Linux/x86
  #endif

#endif


}

Canbus::
~Canbus(void) 
{}

int Canbus::
set_tx_load( const int _ch, const int _precent )
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  if ( _ch == 0 ) {
    delay_reload_can0 = _precent;
    return 0;
  }

  if ( _ch == 1 ) {
    delay_reload_can1 = _precent;
    return 0;
  }

  return -1;
  #else
    return 0;
  #endif
#endif
}
int Canbus::
enable_tx( const int _ch, const int _enable)
{
  printf(" Canbus::enable_tx CH=%i Enable=%i\n",_ch,_enable);
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  if ( _ch == 0 ) {
    m_enable_tx_can0 = _enable;
    return 0;
  }

  if ( _ch == 1 ) {
    m_enable_tx_can1 = _enable;
    return 0;
  }
  return -1;
  #else
    return 0;
  #endif
#endif
}

int Canbus::
send_message( const char* _buf,const int _size )
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  struct can_frame frame;
  memcpy((char*)&frame,
         _buf,
         _size );
  m_msgs.push(frame);
  #else
  return 0;
  #endif
#endif
  return 0;
}
/*
int Canbus::
sendMessage( int _pgn,QByteArray& _ba )
{
  int size = _ba.count();
  // 待发送内容小于8字节的，用单包发送，否则用多包发送
  if( size <= 8 ){
    //singel packet
    int id = 0x18000000 | (_pgn<<8) | (0x00000028);
    int dlc = 8;
    uint8_t dat[8];
    memset((char*)dat,0xFF,8);
    memcpy( (char*)dat ,(char*)_ba.data(),size);
    transmit( id,dlc,dat);
  } else {
    //multi packet
    int sizeof_packet = size / 7;
    if ( (size % 7) != 0 ){
        sizeof_packet++;
    }
    // 多包启动帧
    int id = 0x18ECF928;
    int dlc = 8;
    uint8_t dat[8];
    dat[0] = 32;
    dat[1] = ( size & 0x00FF);      //size_of_byte
    dat[2] = ((size & 0xFF00)>>8); //size_of_byte
    dat[3] = sizeof_packet;         // sizeof packet
    dat[4] = 0xFF;                  // 0xFF
    dat[5] = ( _pgn & 0x0000FF);    // pgn LSB
    dat[6] = ((_pgn & 0x00FF00) >> 8);
    dat[7] = ((_pgn & 0xFF0000) >> 16); // pgn MSB
    transmit( id,dlc,dat);

    // 多包数据帧
    id = 0x18EBF928;
    dlc = 8;
    for ( int i=0;i<sizeof_packet;i++ ){
        memset((char*)dat,0xFF,8);
        dat[0] = i+1;
        // 当剩余字节数<8时，则覆盖剩余字节，其余为0xFF
        // 否则就覆盖全部8个字节
        memcpy( (char*)&dat[1],
                (char*)( _ba.data() + (7*i)) ,
                ((size-(7*i))>8)?8:(size-(7*i)) );
        transmit( id,dlc,dat);
    }
  }
  return _ba.count();
}
int Canbus::
multipacket_TP_CM_BAM( struct can_frame& _can_frm )
{
  // 控制字必须为32
  if ( _can_frm.data[0] != 32 ){
    //Debug::log(_DEBUG_WARRING," MultiPacket message without 0x20 command\n");
    return -1;
  }
  int sa = _can_frm.can_id & 0x000000FF ;

  int packet_delare = 0;
  packet_delare     = _can_frm.data[3];

  int byte_delare   = 0;
  byte_delare       = _can_frm.data[2];
  byte_delare       = ((byte_delare<<8) | _can_frm.data[1]);

  int pgn_delare    = 0;
  pgn_delare        = _can_frm.data[7];
  pgn_delare        = ((pgn_delare<<8) | _can_frm.data[6]);
  pgn_delare        = ((pgn_delare<<8) | _can_frm.data[5]);

  m_mp.sa            = sa;
  m_mp.pgn           = pgn_delare;
  m_mp.byte_delare   = byte_delare;
  m_mp.packet_delare = packet_delare;
  m_mp.byte_receive  = 0;
  m_mp.packet_receive= 0;
  memset((uint8_t*)&m_mp.data,0,2048);
  return 1;
}

int Canbus::
multipacket_TP_DT( struct can_frame& _can_frm  )
{
  int sa = _can_frm.can_id & 0x000000FF ;
  if ( sa != m_mp.sa ){
    return -1;
  }

  if ( (uint8_t*)&m_mp.data == (void*)NULL ){
    printf("m_data == (char*)NULL\n");
    return -1;
  }
  if (m_mp.byte_receive >= 2040 ){
    printf("m_mp->m_byte_receive >= 2040\n");
    return -1;
  }
  memcpy( (uint8_t*)((uint8_t*)&m_mp.data + m_mp.byte_receive ),
          _can_frm.data,
          _can_frm.can_dlc);

  m_mp.byte_receive += _can_frm.can_dlc;
  m_mp.packet_receive++;
  //printf("_can_frm.data {");
  //for( int i=0;i<_can_frm.can_dlc ;i++)
  //    printf("%02x ",*((uint8_t*)&_can_frm.data +i));
  //printf("}\n");
  if ( m_mp.packet_receive == m_mp.packet_delare ){
      // 接收完成后整理数据，剔除包序号等
      //Debug::log( _DEBUG_DETAIL, " TP.DT Completed (b=%i(%i) p=%i(%i) pgn=0x%04x)\n",
                  m_mp.byte_receive,
                  m_mp.byte_delare,
                  m_mp.packet_receive,
                  m_mp.packet_delare,
                  m_mp.pgn);
      //printf("RAW MP_Data {");
      //for( int i=0;i<mp_dev->m_byte_receive;i++)
      //    printf("%02x ",*((uint8_t*)&mp_dev->m_data +i));
      //printf("}\n");
      // 按顺序整理消息内容，并剔除序号
      for ( int p=0; p < m_mp.packet_receive ; p++ ){
          uint8_t packet_index = *((uint8_t*)&m_mp.data + p*8) - 1;
          for ( int b=0; b < 7; b++ ){
              *((uint8_t*)&m_mp.data + packet_index*7 + b) = 
              *((uint8_t*)&m_mp.data + p*8 + b + 1) ;
          }
      }
      emit sig_net_multipacket_msg( m_mp.sa ,
                                    m_mp.pgn,
                                    (uint8_t*)&m_mp.data,
                                    m_mp.byte_delare);
      //printf("MP_Data Sorted {");
      //for( int i=0;i<mp_dev->m_byte_delare;i++)
      //    printf("%02x ",*((uint8_t*)&mp_dev->m_data +i));
      //printf("}\n");
      m_mp.byte_receive   = 0;
      m_mp.packet_receive = 0;
      return 1;
  }
  return 0;
}
int Canbus::
frameCount( int *_rx,int *_tx)
{
  *_rx = m_rx_cnt;
  *_tx = m_tx_cnt;
  return 0;
}

typedef struct mp {
  int     sa;
  int     pgn;
  int     byte_delare;
  int     byte_receive;
  int     packet_delare;
  int     packet_receive;
  uint8_t data[2048];
} MultiPacket_t;
MultiPacket_t m_mp;
int Canbus::

getUdsConnection(void)
{
  return m_uds_connect;
}
*/
/// end of code ///
