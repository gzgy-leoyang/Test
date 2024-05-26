#include "support/yq_rtc.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int get_rtc( struct rtc_time *_rtc )
{
  int rtc_fd;
  struct rtc_time rtc_tm;
  // 打开RTC设备文件
  rtc_fd = open("/dev/rtc", O_RDWR);
  if (rtc_fd == -1) {
      perror("Failed to open /dev/rtc");
      return -1;
  }

  // 读取RTC时间
  if (ioctl(rtc_fd, RTC_RD_TIME, &rtc_tm) == -1) {
      perror("Failed to read RTC time");
      close(rtc_fd);
      return -1;
  }
  memcpy( _rtc,&rtc_tm,sizeof(struct rtc_time) );
  close(rtc_fd);
  return 0;
}

int set_rtc( struct rtc_time *_rtc )
{
  int rtc_fd;
  struct rtc_time rtc_tm;
  memcpy( &rtc_tm,_rtc,sizeof(struct rtc_time) );
  // 打开RTC设备文件
  rtc_fd = open("/dev/rtc", O_RDWR);
  if (rtc_fd == -1) {
      perror("Failed to open /dev/rtc");
      return -1;
  }

  // 读取RTC时间
  if (ioctl(rtc_fd, RTC_SET_TIME, &rtc_tm) == -1) {
      perror("Failed to read RTC time");
      close(rtc_fd);
      return -1;
  }
  close(rtc_fd);
  return 0;
}

/// end of code ///

/* RTC编程
The /dev/rtc (or /dev/rtc0, /dev/rtc1, etc.)  device can be
opened only once (until it is closed) and it is read-only. 

// 头文件
#include <linux/rtc.h>

// 设备文件
1. /dev/rtc, /dev/rtc0, /dev/rtc1, etc.
   RTC special character device files.

2. /proc/driver/rtc
   status of the (first) RTC

// 控制函数
int ioctl(fd, RTC_request, param);

1. RTC_RD_TIME
   Returns this RTC's time in the following structure:

2. RTC_SET_TIME
   Sets this RTC's time to the time specified by the rtc_time
   structure pointed to by the third ioctl(2) argument.  To
   set the RTC's time the process must be privileged (i.e.,
   have the CAP_SYS_TIME capability).

3. 其他命令
   RTC_ALM_READ
   RTC_ALM_SET
   RTC_IRQP_READ
   RTC_IRQP_SET
   RTC_AIE_ON
   RTC_AIE_OFF
   RTC_UIE_ON
   RTC_UIE_OFF
   RTC_PIE_ON
   RTC_PIE_OFF
   RTC_EPOCH_READ
   RTC_EPOCH_SET
   RTC_WKALM_RD
   RTC_WKALM_SET

3. 数据结构
   The fields in this structure have the same meaning and
   ranges as for the tm structure described in gmtime(3).  A
   pointer to this structure should be passed as the third
   ioctl(2) argument.

   struct rtc_time {
       int tm_sec;
       int tm_min;
       int tm_hour;
       int tm_mday;
       int tm_mon;
       int tm_year;
       int tm_wday;     /// unused
       int tm_yday;     // unused
       int tm_isdst;    // unused
   };
   struct rtc_wkalrm {
       unsigned char enabled;
       unsigned char pending;
       struct rtc_time time;
   };
*/
