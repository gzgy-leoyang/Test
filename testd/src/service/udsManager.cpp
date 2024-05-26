#include "service/udsManager.h"

#include "adapter/device_adapter.h"
#include "support/debug_out.h"
#include "main.h"

//////////////////////////////////////////////////
/// PUBLIC
// 初始化就构造管理者,唯一的静态对象
// 饿汉模式,线程安全
UdsManager* UdsManager::manager = new UdsManager();

UdsManager* UdsManager::
getInstance() 
{
  return manager;
};

int UdsManager::
addNetDev( Abstract_net* _net ) 
{
  m_net = _net;
  return 0;
}

int UdsManager::
setLocalAddress( char _addr)
{
  m_local_addr = _addr ;
  return 0;
}

int UdsManager::
setTesterAddress( char _addr)
{
  m_tester_addr = _addr;
  return 0;
}

int UdsManager::
registeGeneralOperatorWithDid( int _did,UDS_operator_t& _op )
{
  if ( _op.callback == nullptr ) {
    LOGE<<" The pointer of Callback is nullptr";
    return -1;
  }
  m_general_ops.insert(std::pair<int, UDS_operator_t>(_did,_op) );
  return 0;
}

///////////////////////////////////////////////////
/// PRIVATE

UdsManager::
UdsManager()
{
  m_session_sta      = 0;
  m_safetyAccess_sta = 0;
  m_local_addr       = 0x01;
  m_tester_addr      = 0x00;
  m_net              = nullptr;
}

UdsManager::
~UdsManager()
{
}


/////////////////////////////////////////////////
/// UDS协议层入口 
int UdsManager::
messageHandler( uint8_t* _buf,uint8_t _size )
{
  return netlayout_handle(_buf,_size);
}

/////////////////////////////////////////////////
/// 网络层控制

// 网络层处理,主要是单帧/多帧的处理
int UdsManager::
netlayout_handle(uint8_t* _buf,uint8_t _size)
{
#if _LONG_DEBUG_INFO
  printf(" UdsManager::netlayout_handle <");
  for (int i=0;i<8;i++ ){
    printf("%02x ",*(_buf+i) );
  }
  printf(">\n");
#endif

  int ret        = 0;
  int type = ((*(_buf+0) & 0xF0)>>4);
  if ( type ) {
    // 多帧,首帧
    ret = multi_frame(_buf,_size);
  } else {
    // 单帧
    ret = singel_frame(_buf,_size);
  }
  return ret;
}

int UdsManager::
singel_frame(uint8_t* _buf,uint8_t _size)
{
#if _LONG_DEBUG_INFO
  printf(" UdsManager::singel_frame <");
  for (int i=0;i<8;i++ ){
    printf("%02x ",*(_buf+i) );
  }
  printf(">\n");
#endif

  int ret = 0;
  uint8_t sid = *(_buf+1) ;
  switch ( sid ) {
    case 0x10:
      ret = sessionControl_10(_buf,_size);
      break;
    case 0x22:
    case 0x2E:
      // 读写参数
      ret = generalDataOperat( _buf,_size);
      break;
    case 0x27:
      ret = safetyAccess_27(_buf,_size) ;
      break;
    case 0x11:
      // ECU reset
      //ret = resetECU_11(reinterpret_cast<uint8_t*>(_buf+1),_size-1);
      break;
    //case 0x2E:
      // 写参数
      //ret = writeData_2E(reinterpret_cast<uint8_t*>(_buf+1),_size-1);
     // break;
  }

  if (ret<0){
    // 负应答,返回字节数,取反可用
    *(_buf+0) = 0x03;
    *(_buf+1) = 0x7F; // 负应答标记
    *(_buf+2) = sid; // 重复SID
    *(_buf+3) = 0x10; // NRC负应答原因
    ret = -ret;
  } else {
    // 正应答
    *(_buf+0)  = ret+3; // 答应数据长度
    *(_buf+1) += 0x40;  // 正应答,
  }
  // 发出应答
  int addr = m_tester_addr;
  addr     = (addr << 8) | m_local_addr;
  Net_can* can = static_cast< Net_can* >( m_net );
  can->transmit_message(0x0CEF0000|addr, 
                        reinterpret_cast<char*>(_buf),
                        8);
#if _LONG_DEBUG_INFO
  printf(" UdsManager::singel_frame,Respond [");
  for (int i=0;i<8;i++ ){
    printf("%02x ",*(_buf+i) );
  }
  printf("]\n");
#endif
  return ret;
}


int UdsManager::
multi_frame(uint8_t* _buf,uint8_t _size)
{
  static int statue =0;// 状态控制标志,静态
  return 0;
}

/// 结束:网络层控制
/////////////////////////////////////////////////

/////////////////////////////////////////////////
/// 单帧服务
/// 1. 通用数据读写
int UdsManager::
generalDataOperat(uint8_t* _buf,uint8_t _size )
{
  int ret      = 0;
  uint16_t did = *(_buf+2);
  did          = (did<<8)|(*(_buf+3));
  LOGV<<"General Data DID:"<<std::hex<<did;
  try {
    UDS_operator_t op = m_general_ops.at(did);
    if ( (op.session == m_session_sta     ) && 
         (op.safety  == m_safetyAccess_sta)) {
      ret = op.callback( reinterpret_cast<char*>(_buf),_size);
    } else {
      LOGD<<" The condition of operation is NOT ";
    }
  } catch ( const std::out_of_range &exc) {
    LOGW<<" DID is out of range..."<<did;
  }
  return ret ;
}

/// 2. 会话控制
int UdsManager::
sessionControl_10( uint8_t* _buf,uint8_t _size )
{
  UNUSED(_size);
#if _LONG_DEBUG_INFO
  printf("  Session Control: ");
  for ( int i=0;i<_size;i++ ) {
    printf(" %02x",*(_buf+i));
  }
  printf("\n");
#endif

  int session = *(_buf+2);
  LOGI<<"Session control:"<<session;
  switch ( *(_buf+2) ) {
    case 0x60:
      // 供应商自定义会话:0x60=YQ
      *(_buf+2)     = 0x60; // 子功能码
      *(_buf+3)     = 0x00;
      *(_buf+4)     = 0x3C; // 处理延迟60s
      *(_buf+5)     = 0x00;
      *(_buf+6)     = 0xF0; // 处理过程最大延迟240s
      m_session_sta = 0x60;// 切换会话
      LOGI<<" Session control..."<<m_session_sta<<" Safety Access..."<<m_safetyAccess_sta;
      return 5;
    case 0x00:
      // 返回默认会话
      *(_buf+2)     = 0x00; // 子功能码
      *(_buf+3)     = 0x00;
      *(_buf+4)     = 0x00; // 处理延迟60s
      *(_buf+5)     = 0x00;
      *(_buf+6)     = 0x00; // 处理过程最大延迟240s
      m_session_sta = 0x00;
      LOGI<<" Session control..."<<m_session_sta<<" Safety Access..."<<m_safetyAccess_sta;
      // Todo: 返回默认会话时,应该不需要提供延迟时间
      // 那回复数据长度是否不需要包含的4个字节
      return 5;
    default:
      LOGW<<" Undefine Session :"<<session;
      return -3;
  }
}

int UdsManager::
generateKeySeed( uint8_t* _seed,uint8_t _size )
{
  UNUSED(_size);
  // 生成密码种子,取系统时间
  *(_seed+0) = 0x01;
  *(_seed+1) = 0x02;
  *(_seed+2) = 0x03;
  *(_seed+3) = 0x04;
  return 0;
}

// 由种子,经过本地计算获取密码
int UdsManager::
generateKey( uint8_t *_seed, uint8_t* _key_e,uint8_t _size )
{
  UNUSED(_size);
  // 简单密码
  *(_key_e+0) = *(_seed+0);
  *(_key_e+1) = *(_seed+1);
  *(_key_e+2) = *(_seed+2);
  *(_key_e+3) = *(_seed+3);
  return 0;
}

int UdsManager::
checkKey( uint8_t* _key_e,uint8_t *_key_t,uint8_t _size )
{
  for ( int i=0;i<_size;i++ ) {
    if ( *(_key_e+i) != *(_key_t+i) ) {
      return -1;
    }
  }
  return 0;
}

int UdsManager::
safetyAccess_27( uint8_t* _buf,uint8_t _size )
{
  UNUSED(_size);
  static uint8_t seed[4];
  static uint8_t key_e[4];
  static int request_seed = 0;

#if _DEBUG_INFO
  //printf("  Safety Accesss: ");
  //for ( int i=0;i<_size;i++ ) {
  //  printf(" %02x",*(_buf+i));
  //}
  //printf("\n");
#endif

  if ( m_session_sta != 0x60 ) {
    return 0;
  }

  switch ( *(_buf+2) ) {
    case 0x03:
      LOGI<<" Safety Access...request seed of key";
      // 请求密钥种子,准备种子后发出
      generateKeySeed( seed,4 );
      generateKey(seed,key_e,4);
      memcpy (reinterpret_cast<char*>(_buf+3),
              reinterpret_cast<char*>(seed),
              4);
      request_seed= 1;
      LOGI<<" Session control..."<<m_session_sta<<" Safety Access..."<<m_safetyAccess_sta;
      return 5;
    case 0x04:
      LOGI<<" Safety Access...check key";
      // 收到密码,准备校验
      if ( request_seed == 1 ) {
        if (checkKey((key_e),(_buf+3),4) < 0 ){
          // 提示上层作出负应答
          m_safetyAccess_sta = 0; // 默认会话
          LOGI<<" Safety Access...ERROR";
          LOGI<<" Session control..."<<m_session_sta<<" Safety Access..."<<m_safetyAccess_sta;
          LOGI;
          return -3;
        } else {
          m_safetyAccess_sta = 1; // 默认会话
          memset( reinterpret_cast<char*>(_buf+3),
                  0xAA,
                  4);
          LOGI<<" Safety Access...OK";
          LOGI<<" Session control..."<<m_session_sta<<" Safety Access..."<<m_safetyAccess_sta;
          return 1;
        }
      }
      break;
    default:
      break;
  }
  return 0;
}

int UdsManager::
resetECU_11(uint8_t* _buf,int _size)
{
  UNUSED(_buf);
  UNUSED(_size);
#if _LONG_DEBUG_INFO
  //printf("  Reset ECU: ");
  //for ( int i=0;i<_size;i++ ) {
  //  printf(" %02x",*(_buf+i));
  //}
  //printf("\n");
#endif

  if ( m_safetyAccess_sta == 1 ) {
#if _LONG_DEBUG_INFO
    printf(" Reseting...\n");
#endif
    //reboot(LINUX_REBOOT_CMD_RESTART);
    return 0;
  } else {

    printf("  Failed to reset ECU without SafetyAccess mode\n");
    return -3;
  }
}


/// end of code ///
