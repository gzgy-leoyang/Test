#ifndef UDS_MANAGER_H
#define UDS_MANAGER_H

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <functional>
#include <map>

#include "../net/net_can.h"

typedef struct {
  int did;
  int safety;
  int (*callBack)( uint8_t* _buf,int _size );
} Did_handler_t;

typedef struct {
  int safety;
  int session;
  std::function< int(char* ,int) > callback;
} UDS_operator_t;

class UdsManager
{
  public:
    static UdsManager* getInstance() ;

    int messageHandler(uint8_t* _buf,uint8_t _size);
    int addNetDev( Abstract_net* _net );
    int setLocalAddress( char _addr);
    int setTesterAddress( char _addr);
    int registeGeneralOperatorWithDid( int _did,UDS_operator_t& _op );
  private:
    static UdsManager* manager;

    UdsManager();
    ~UdsManager();

    int ackMessage(uint8_t* _buf,uint8_t _size);
    // 外部模块读写
    int generalDataOperat(uint8_t* _buf,uint8_t _size );
    // UDS 内部流程控制
    int sessionControl_10( uint8_t* _buf,uint8_t _size );
    int resetECU_11(uint8_t* _buf,int _size);
    int safetyAccess_27( uint8_t* _buf,uint8_t _size );
    int generateKeySeed( uint8_t* _seed,uint8_t _size );
    int generateKey( uint8_t *_seed, uint8_t* _key_e,uint8_t _size );
    int checkKey( uint8_t* _key_e,uint8_t *_key_t,uint8_t _size );

    // 网络层处理,主要是单帧/多帧的处理
    int netlayout_handle(uint8_t* _buf,uint8_t _size);
    int singel_frame(uint8_t* _buf,uint8_t _size);
    int multi_frame(uint8_t* _buf,uint8_t _size);
    // 记录通用数据操作列表
    std::map<int, UDS_operator_t> m_general_ops;
    // 在UDS层直接发出
    Abstract_net* m_net;
    // 标记UDS内部状态
    int m_session_sta      ;
    int m_safetyAccess_sta ;
    // 标记发出应答时使用的地址
    char m_local_addr;
    char m_tester_addr;
};
#endif 
