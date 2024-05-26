#ifndef NET_CAN_H
#define NET_CAN_H

#include "abstract_net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/uinput.h>

#include <thread>
#include <map>

#include "../support/threadpool.h"


typedef int (*callback_t) ( char*,int );

class Net_can : public Abstract_net 
{
public:
    explicit Net_can( const char* _can="can0");
    Net_can( const char _addr,const char _ta, const char* _can);

    Net_can( const char _addr,
             const char _ta,
             const char* _can="can0",
             const int   _ts=0);  // 线程池规模,>0时,构造线程池

    int transmit_message( const int& _info,
                                char* _buf,
                          const int& _size) override ;
    int register_callback_for_ID( int _id, callback_t _cb );

private:
    void callback_receive(void);

    callback_t    m_cb;
    ThreadPool*   m_pool;
    char          m_this_address;
    char          m_tester_address;
    int           m_fd;
    std::thread   m_p1;
    int           m_can_x;
    int           m_id_filted;
    char          m_ifname[20];
    struct        ifreq           m_ifreq;
    struct        sockaddr_can    m_canAddr;

    std::map<int, callback_t> m_ID_callback ;
};
#endif
