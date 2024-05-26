#ifdef __cplusplus
extern "C"{
#endif

  #ifndef YQ_RTC_H
  #define YQ_RTC_H

  #include <unistd.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <fcntl.h>
  #include <sys/ioctl.h>
  #include <linux/rtc.h>

  extern int get_rtc( struct rtc_time *_rtc );
  extern int set_rtc( struct rtc_time *_rtc );
  #endif

#ifdef __cplusplus
}
#endif
