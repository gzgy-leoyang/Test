#include "support/watchdog.h"
#include <sys/ioctl.h>
#include <linux/watchdog.h>

int watchdogSetTimeout( const char* _dog, const int _ms );
int watchdogStart( const char* _dog );
int watchdogResetTimeout( const char* _dog );

int watchdogStart( const char* _dog)
{
  int fd = open(_dog,O_RDWR);
  if (fd < 0) {
    printf("Failed to open device\n");
    return -1;
  }
  ioctl(fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);
  close(fd);
  return 0;
}

int watchdogSetTimeout( const char* _dog, const int _sec )
{
  int fd = open(_dog,O_RDWR);
  if (fd < 0) {
    printf("Failed to open device\n");
    return -1;
  }
  ioctl(fd, WDIOC_SETTIMEOUT,&_sec);
  ioctl(fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);
  close(fd);
  return 0;
}

int watchdogResetTimeout( const char* _dog)
{
  int fd = open(_dog,O_RDWR);
  if (fd < 0) {
    printf("Failed to open device\n");
    return -1;
  }
  ioctl(fd, WDIOC_KEEPALIVE, 0);
  close(fd);
  return 0;
}
/// end of code ///
