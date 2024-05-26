#include "adapter/hyman_adapter.h"
#include "support/debug_out.h"
#include "service/udsManager.h"
#include "main.h"

#include <iostream>
#include <fstream>

using std::ifstream;
using std::ios;

///////////////
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/file.h>

//typedef struct{
//  int poweron_hour;
//  int poweron_hour_check;
//  int idle_hour;
//  int idle_hour_check;
//  int working_hour;
//  int working_hour_check;
//  int movement_hour;
//  int movement_hour_check;
//  int B1_hour;
//  int B1_hour_check;
//  int B2_hour;
//  int B2_hour_check;
//  int B3_hour;
//  int B3_hour_check;
//  int B4_hour;
//  int B4_hour_check;
//  int Att1_hour;
//  int Att1_hour_check;
//  int Att2_hour;
//  int Att2_hour_check;
//  int Att3_hour;
//  int Att3_hour_check;
//  int Att4_hour;
//  int Att4_hour_check;
//  int phase_hour;
//  int phase_hour_check;
//  int idle_speed;
//  int idle_speed_check;
//
//  int P_hour;
//  int P_hour_check;
//  int E_hour;
//  int E_hour_check;
//  int L_hour;
//  int L_hour_check;
//}Hours_raw_t;

typedef struct Hours_t{
	int poweron_hour;
	int idle_hour;
	int working_hour;
	int movement_hour;

	int B1_hour;
  int B2_hour;
  int B3_hour;
  int B4_hour;

  int Att1_hour;
  int Att2_hour;
  int Att3_hour;
  int Att4_hour;

  int phase_hour;
  int idle_speed;

  int P_hour;
  int E_hour;
  int L_hour;

	char version_info[20];
  int cmp_reminder[4];
}Hours_t;

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* arrary;
    struct seminfo* __buf;
};
int init_shm(int *_sem_id,
             int *_shm_id,
             char *_shm_seg,
             const int _size );

int clean_shm(const uint8_t _sem,
              char *_seg,
              const int _size );

int get_shm(const uint8_t _sem,
            const char *_seg, 
            uint8_t *_buf,
            const int _size );

int   g_sem_id    = 0;
int   g_shm_id    = 0;
char* g_shm_seg   = NULL;

////////////////////////////////

Hyman_adapter::
Hyman_adapter():Adapter()
{
  UdsManager* um = UdsManager::getInstance();

  UDS_operator_t op;
  op.safety=0;
  op.session=0;
  op.callback=getSecFromHyman_x;
  um->registeGeneralOperatorWithDid(0x8400,op);

  op.callback=getSecFromHyman;
  um->registeGeneralOperatorWithDid(0x8401,op);

  op.callback=getSecFromHourmeter;
  um->registeGeneralOperatorWithDid(0x8402,op);


  /// 准备读取hourmeter_qt
  if ( init_shm( &g_sem_id,
                 &g_shm_id,
                 g_shm_seg,
                 sizeof(Hours_t)) < 0 ){
    printf(" Failed to initialize shared memory\n");
  }
  clean_shm(g_sem_id,
            g_shm_seg,
            sizeof(Hours_t));
  /// 完成读取
}

int Hyman_adapter::
getSecFromHyman_x( char* _buf,int _size )
{
  UNUSED(_size);
  int sec = 0;
  if ( readIntFromFile( "/run/Hyman_work/second",
                        sec ) < 0 ) {
    LOGW<<" Failed to get second from file";
    return 0;
  }
  LOGI<<" Hyman_work Sec : "<<sec;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = (  sec & 0xFF000000) >> 24;
  *(_buf+5) = (  sec & 0x00FF0000) >> 16;
  *(_buf+6) = (  sec & 0x0000FF00) >> 8;
  *(_buf+7) = (  sec & 0x000000FF);
  return 4;
}

int Hyman_adapter::
getSecFromHyman( char* _buf,int _size )
{
  UNUSED(_size);
  int sec = 0;
  if ( readIntFromFile( "/tmp/Hyman/second",
                        sec ) < 0 ) {
    LOGW<<" Failed to get second from file";
    return 0;
  }
  LOGI<<" Hyman Sec : "<<sec;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = (  sec & 0xFF000000) >> 24;
  *(_buf+5) = (  sec & 0x00FF0000) >> 16;
  *(_buf+6) = (  sec & 0x0000FF00) >> 8;
  *(_buf+7) = (  sec & 0x000000FF);
  return 4;
}

int Hyman_adapter::
getSecFromHourmeter( char* _buf,int _size )
{
  UNUSED(_size);

  Hours_t ht;
  if (get_shm( g_sem_id, 
               g_shm_seg,
               reinterpret_cast<uint8_t*>(&ht),
               sizeof(Hours_t)) < 0) {
  }

  int sec = ht.poweron_hour ;

  LOGI<<" Hourmeter Sec: "<<sec;
  std::memset((_buf+4),0xAA,4);
  *(_buf+4) = (  sec & 0xFF000000) >> 24;
  *(_buf+5) = (  sec & 0x00FF0000) >> 16;
  *(_buf+6) = (  sec & 0x0000FF00) >> 8;
  *(_buf+7) = (  sec & 0x000000FF);
  return 4;
}


/////////////////
/// 以下用于读取 hourmeter_qt 数据


int init_shm(int*  _sem_id,
             int*  _shm_id,
             char* _shm_seg,
             const int _size )
{
  UNUSED(_shm_seg);
  UNUSED(_shm_id);
  UNUSED(_sem_id);
  //_shm_seg = NULL; 
  int key = ftok ( "/dev/null",6);
  int sem_id = semget(key,
                      1,
                      IPC_CREAT | 0666 );
  if ( sem_id < 0 ){
    LOGE<<" Fail to get SEMAPHORE_ID";
    return -1;
  }
  g_sem_id = sem_id;
  LOGV<<" SEMAPHORE_ID:"<<sem_id;

  union semun sem_union;
  sem_union.val = 1;
  semctl(sem_id, 0, SETVAL, sem_union);

  int shm_id = shmget(key,_size,IPC_CREAT);
  if ( shm_id < 0 ){
    LOGW<<" Fail to attach Share_Memory,try to reboot";
    return -1;
  }
  g_shm_id = shm_id;
  LOGV<<" ShareMem_ID:"<<shm_id;
  char* seg = reinterpret_cast<char*>(shmat(g_shm_id,
                           NULL,
                           SHM_R | SHM_W));
  if ( seg != (void*)NULL ){
    //memset(seg ,0,_size);
    g_shm_seg = seg;
    LOGV<<" ShareMem...OK";
    return 0;
  } else {
    LOGW<<" Fail to attach SHARM_MEM";
    return -1;
  }
}

int clean_shm(const uint8_t _sem_id,
              char *_seg,
              const int _size )
{
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1;
  sem_b.sem_flg = SEM_UNDO;
  if(semop(_sem_id, &sem_b, 1) == -1){
    LOGW<<" semaphore_p failed";
    return -1;
  }
  memset(reinterpret_cast<char*>(_seg),0,_size);
  //sem_v(_sem);
  sem_b.sem_num = 0;
  sem_b.sem_op = 1;
  sem_b.sem_flg = SEM_UNDO;
  if(semop(_sem_id, &sem_b, 1) == -1){
    LOGW<<" Semaphore_v failed";
    return -1;
  }
  return 0;
}

int get_shm(const uint8_t _sem_id,
            const char *_seg, 
            uint8_t *_buf,
            const int _size )
{
  //sem_p(_sem);
  struct sembuf sem_b;
  sem_b.sem_num = 0;
  sem_b.sem_op = -1;
  sem_b.sem_flg = SEM_UNDO;
  if(semop(_sem_id, &sem_b, 1) == -1){
    LOGW<<" Semaphore_p failed";
    return -1;
  }
  memcpy(_buf,_seg,_size);
  //sem_v(_sem);
  sem_b.sem_num = 0;
  sem_b.sem_op = 1;
  sem_b.sem_flg = SEM_UNDO;
  if(semop(_sem_id, &sem_b, 1) == -1){
    LOGW<<" Semaphore_v failed";
    return -1;
  }
  return 0;
}
/// 完成:读取hourmeter_qt数据

/// end of code ///
