#ifdef __cplusplus
extern "C"{
#endif

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
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

extern int watchdogSetTimeout( const char* _dog, const int _ms );
extern int watchdogStart( const char* _dog );
extern int watchdogResetTimeout( const char* _dog );

#endif

#ifdef __cplusplus
}
#endif
