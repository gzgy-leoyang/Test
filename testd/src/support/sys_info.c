#include "support/sys_info.h"
#include "support/c_debug_out.h"
//#include "main.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static struct sysinfo m_sys_info;

int sysInfo_getCpuID(long long unsigned int* const _id)
{
  char    buffer[512]    = {'\0'};
  //char    file_name[512] ;

  FILE*   fd             = NULL;
  fd = fopen("/proc/cpuinfo" ,"r" );
  if ( fd == NULL ) {
    return -1;
  } else {
    long long unsigned int id = 0;
    do {
      if ( fgets(buffer, ARRAY_SIZE(buffer), fd) == NULL ) {
        return -1;
      }
    }while ( !strstr(buffer,"Serial" )) ;
    //char* pstr = fgets(buffer, ARRAY_SIZE(buffer), fd);
    //while ( !strstr(buffer,"Serial" )) {
    //  pstr = fgets(buffer, ARRAY_SIZE(buffer), fd);
    //}

    sscanf( buffer,"Serial\t: %llx", &id);
    fclose(fd);
    //CLOGD(" Cpu ID: %i",id);
    *_id = id;
    return 0;
  }
  return -1;
}

int sysInfo_getUptime( long* const _uptime ) 
{
  if( sysinfo(&m_sys_info) < 0 ) {
  } else {
    *_uptime = m_sys_info.uptime;
    //CLOGD(" Uptime: %li",*_uptime);
  }
  return 0;
}
/// 平均负载 /// 
void sysInfo_getLoadAvg( float* _f1, float* _f2,float* _f3)
{
  //某文中发现的一个神秘系数(2<<16)
  const float load_avg_sysinfo_scale = 2<<15;
  if( sysinfo(&m_sys_info) < 0 ) {
  } else {
    *_f1 = m_sys_info.loads[0]/load_avg_sysinfo_scale;
    *_f2 = m_sys_info.loads[1]/load_avg_sysinfo_scale;
    *_f3 = m_sys_info.loads[2]/load_avg_sysinfo_scale;
  }
  return ;
}

int sysInfo_getLoadAvg_1min( int* const _load ) 
{
  //某文中发现的一个神秘系数(2<<16)
  const float load_avg_sysinfo_scale = 2<<15;
  if( sysinfo(&m_sys_info) < 0 ) {
  } else {
    float load_val = m_sys_info.loads[0]/load_avg_sysinfo_scale;

    *_load = (int)(load_val*100);
    //CLOGD(" LoadAvg(1min): %.02f %i",load_val,*_load);
  }
  return 0;
}

int sysInfo_getLoadAvg_5min( int* const  _load )
{
  //某文中发现的一个神秘系数(2<<16)
  const float load_avg_sysinfo_scale = 2<<15;
  if( sysinfo(&m_sys_info) < 0 ) {
  } else {
    float load_val = m_sys_info.loads[1]/load_avg_sysinfo_scale;
    *_load = (int)(load_val*100);
    //CLOGD(" LoadAvg(5min): %.02f %i",load_val,*_load);
  }
  return 0;
}

int sysInfo_getLoadAvg_15min( int* const _load )
{
  //某文中发现的一个神秘系数(2<<16)
  const float load_avg_sysinfo_scale = 2<<15;
  if( sysinfo(&m_sys_info) < 0 ) {
  } else {
    float load_val = m_sys_info.loads[2]/load_avg_sysinfo_scale;
    *_load = (int)(load_val*100);
    //CLOGD(" LoadAvg(15min): %.02f %i",load_val,*_load);
  }
  return 0;
}
/// 平均负载 /// 

int sysInfo_getCpuFreq( const int _cpu, unsigned int* const _freq )
{
  char    buffer[512]    = {'\0'};
  char    file_name[512] ;
  sprintf( file_name,
           "/sys/devices/system/cpu/cpu%i/cpufreq/cpuinfo_cur_freq",
           _cpu);

  FILE*   fd             = NULL;
  fd = fopen( file_name ,"r" );
  if ( fd == NULL ) {
    return -1;
  } else {
    unsigned int freq = 0;
    if ( fgets(buffer, ARRAY_SIZE(buffer), fd) != NULL ) {
      sscanf( buffer,"%u", &freq);
      *_freq = freq/1000;
      //CLOGD(" Cpu Freq: %iMHz",*_freq);
    }
    fclose(fd);
    return 0;
  }
  return -1;
}

int sysInfo_getTemp( int *_t1,int *_t2 )
{
  char    buffer[512]    = {'\0'};
  FILE*   fd             = NULL;

  static int t1 = 0;
  static int t2 = 0;

  fd = fopen("/sys/class/thermal/thermal_zone0/temp" ,
             "r" );
  if ( fd != NULL ) {
    if (fgets(buffer, ARRAY_SIZE(buffer), fd) != NULL) {
      sscanf( buffer,"%i", &t1);
      *_t1 = t1/1000;
      //CLOGD(" Cpu Temp1: %i",*_t1);
    }
    fclose(fd);
  }

  fd = fopen("/sys/class/thermal/thermal_zone1/temp" ,
             "r" );
  if ( fd != NULL ) {
    if ( fgets(buffer, ARRAY_SIZE(buffer), fd) != NULL ) {
      sscanf( buffer,"%i", &t2);
      *_t2 = t2/1000;
      //CLOGD(" Cpu Temp2: %i",*_t2);
    }
    fclose(fd);
  }
  return 0;
}

int sysInfo_getMem( int* const _total,int* const  _avail )
{
  char    buffer[512]    = {'\0'};
  FILE*   fd             = NULL;
  fd = fopen("/proc/meminfo" ,"r" );
  if ( fd == NULL ) {
    return -1;
  } else {
    static int mem_total = 0;
    if ( fgets(buffer, ARRAY_SIZE(buffer), fd) != NULL ) {
      if ( mem_total == 0 ) {
        int ret = sscanf( buffer,"MemTotal: %i kB", &mem_total);
        if ( ret != 1 ){
          mem_total = 0;
        } else {
          //CLOGD(" Total Mem: %iMB",*_total);
        }
      }
    }
    *_total = (mem_total>>10);

    //MemFree
    if ( fgets(buffer, ARRAY_SIZE(buffer), fd) != NULL ) {
    }
    //ret = sscanf( buffer,"MemFree: %u kB", &mem_free);
    //if ( ret != 1 ){
    //  mem_free = 0;
    //}
    //if mem_total > 0 )
    //  *_free_prec = (100 * (double)mem_free/mem_total) ;

    //MemAvailable
    int mem_avail = 0;
    if ( fgets(buffer, ARRAY_SIZE(buffer), fd) != NULL ) {
      int ret = sscanf( buffer,"MemAvailable: %i kB", &mem_avail);
      if ( ret != 1 ){
        mem_avail= 0;
      } else {
        *_avail = (mem_avail >> 10);
        //CLOGD(" Avail Mem: %iMB",*_avail);
      }
    }
    fclose(fd);
  }
  return 0;
}


int sysInfo_getCanRx( const int _port,int* _val )
{
  char    buffer[512]    = {'\0'};
  char    file_name[512] ;
  FILE*   fd             = NULL;

  sprintf( file_name,
           "/sys/class/net/can%i/statistics/rx_packets",
           _port);
  fd = fopen( file_name ,"r" );
  
  if ( fd != NULL ) {
    if ( fgets(buffer, ARRAY_SIZE(buffer), fd) != NULL ) {
      sscanf( buffer,"%i", _val);
      //CLOGD(" CAN_%i RX: %i",_port,*_val);
    }
    fclose(fd);
  } else {
    return -1;
  }
  return 0;
}
int sysInfo_getCanTx( const int _port,int* _val )
{
  char    buffer[512]    = {'\0'};
  char    file_name[512] ;
  FILE*   fd             = NULL;

  sprintf( file_name,
           "/sys/class/net/can%i/statistics/tx_packets",
           _port);
  fd = fopen( file_name ,"r" );
  
  if ( fd != NULL ) {
    if ( fgets(buffer, ARRAY_SIZE(buffer), fd) != NULL ) {
      sscanf( buffer,"%i", _val);
      //CLOGD(" CAN_%i TX: %i",_port,*_val);
    }
    fclose(fd);
  } else {
    return -1;
  }
  return 0;
}

int sysInfo_getInputDeviceEventByName( const char* _name,char* _event)
{
  int     key_flag    = 0;
  char    buffer[512] = {'\0'};
  FILE*   fd          = NULL;
  fd = fopen( "/proc/bus/input/devices","r" );
  if ( fd != NULL ) {
    while (fgets(buffer,512,fd) != NULL){
      if ( buffer[0] == 'N') {
        if (strstr( buffer,_name ) != NULL) {
          key_flag = 1;
        }
      } else if ( buffer[0] == 'H') {
        if ( key_flag == 1 ) {
          char* p = strstr( buffer,"event");
          if ( p != NULL ) {
            char* e = strtok(p," ");
            if ( strcpy( _event,e )  != NULL ) {
              fclose(fd);
              return 0;
            } else {
              fclose(fd);
              return -1;
            }
          }
        }
      } else {
      }
    }
    fclose(fd);
  } else {
    return -1;
  }
  return 0;
}
/// end of code ///
