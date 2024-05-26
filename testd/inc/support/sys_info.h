#ifdef __cplusplus
extern "C"{
#endif

  #ifndef SYS_INFO_H
  #define SYS_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
//#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/syscall.h>

extern int  sysInfo_getTemp( int* _t1,int* _t2 );
extern int  sysInfo_getMem(int* const _total,int* const  _avail);
extern int  sysInfo_getCpuFreq( const int _cpu,unsigned int* const _freq );
extern int  sysInfo_getLoadAvg_1min( int* const _load );
extern int  sysInfo_getLoadAvg_5min( int* const  _load );
extern int  sysInfo_getLoadAvg_15min( int* const  _load );
extern int  sysInfo_getCpuID( long long unsigned int* const _id);
extern int  sysInfo_getUptime( long* const _uptime ) ;
extern int  sysInfo_getCanRx( const int _port,int* _val );
extern int  sysInfo_getCanTx( const int _port,int* _val );

extern int  sysInfo_getInputDeviceEventByName( const char* _key,char* _name );
  #endif // SYS_INFO_H
#ifdef __cplusplus
}
#endif
