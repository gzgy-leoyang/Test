#ifndef C_DEBUG_OUT_H
#define C_DEBUG_OUT_H

#include <stdio.h>

#define _VERBOSE  (6)
#define _DEBUG    (5)
#define _INFO     (4)
#define _WARNING  (3)
#define _ERROR    (2)
#define _FATAL    (1)
#define _NONE     (0)

#pragma GCC diagnostic push

//#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"

// 由于以下变量仅出现在宏定义的函数中,编译器检查会报警告
// 需要具备屏蔽特定的警告
static int debug_level = 0;

#pragma GCC diagnostic pop

#define CLOG_SET_LEVEL( _level )  debug_level=_level

#define CLOGV(fmt,args...) { \
  if( debug_level <= _VERBOSE ) { \
    printf("\33[0;36mVERB [%s@%i] " fmt "\33[0m\n", \
        __FILE__,__LINE__,##args); \
  } \
}

#define CLOGD(fmt,args...) { \
  if( debug_level <= _DEBUG ) \
    printf("\33[0;36mDEBUG [%s@%i] " fmt "\33[0m\n", \
        __FILE__,__LINE__,##args); \
}

#define CLOGI(fmt,args...) { \
  if( debug_level <= _INFO) \
    printf("\33[0;36mINFO [%s@%i] " fmt "\33[0m\n", \
        __FILE__,__LINE__,##args); \
}

#define CLOGW(fmt,args...) { \
  if( debug_level <= _WARNING) \
    printf("\33[0;33mWARNING [%s@%i] " fmt "\33[0m\n", \
        __FILE__,__LINE__,##args); \
}

#define CLOGE(fmt,args...) { \
    printf("\33[1;41;37mERROR [%s@%i] " fmt "\33[0m\n", \
        __FILE__,__LINE__,##args); \
}

#define CLOGF(fmt,args...) { \
    printf("\33[1;41;37mFATAL [%s@%i] " fmt "\33[0m\n", \
        __FILE__,__LINE__,##args); \
}
#endif
