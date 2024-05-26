#include "adapter/device_adapter.h"

#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "support/sys_info.h"
#include "support/c_debug_out.h"
#include "main.h"

static int writeFile( const char* _file_name,
                      char *_buf,
                      const int _size);
int configScreen( const int _w,const int _h,const int _c );

int Device_adapter::m_scr_width  =0;
int Device_adapter::m_scr_height =0;
int Device_adapter::m_scr_color  =0;
int Device_adapter::m_tp_x       =0;
int Device_adapter::m_tp_y       =0;

Device_adapter::
Device_adapter() : Adapter()
{
}

Device_adapter::
~Device_adapter()
{
}

int Device_adapter::
setScreenWidth( char* _buf,int _size ) 
{
  int dat = 0;
  for ( int i=0;i<_size;i++ ) {
    dat = (dat<<8)|(*(_buf+0+i));
  }

  if ( dat < 1920 ) {
    m_scr_width = dat;
  } else {
    return -1;
  }

#if _DEBUG_INFO
  printf("    Set ScreenWidth: ");
  for ( int i=0;i<_size;i++ ) {
    printf(" %02x",*(_buf+i));
  }
  printf(" (%i)\n",dat);
#endif

  if(configScreen( m_scr_width,m_scr_height,m_scr_color )<0) {
    printf(" Failed to  reconfig Screen\n");
  }
  return 0;
}

int Device_adapter::setScreenHeight( char* _buf,int _size ) 
{
  int dat = 0;
  for ( int i=0;i<_size;i++ ) {
    dat = (dat<<8)|(*(_buf+0+i));
  }

  if ( dat < 1920 ) {
    m_scr_height= dat;
  } else {
    printf(" Screen Height error.. %i\n",dat);
    return -1;
  }

#if _DEBUG_INFO
  printf("    Set ScreenHeight: ");
  for ( int i=0;i<_size;i++ ) {
    printf(" %02x",*(_buf+i));
  }
  printf(" (%i)\n",dat);
#endif

  if(configScreen( m_scr_width,m_scr_height,m_scr_color )<0) {
    printf(" Failed to reconfig Screen\n");
  }
  return 0;
}

int Device_adapter::setScreenColor( char* _buf,int _size )
{
  UNUSED(_buf);
  UNUSED(_size);
  return 0;
}

int Device_adapter::setScreenTpX( char* _buf,int _size )
{
  UNUSED(_buf);
  UNUSED(_size);
  return 0;
}

int Device_adapter::setScreenTpY( char* _buf,int _size )
{
  UNUSED(_buf);
  UNUSED(_size);
  return 0;
}

int configScreen( const int _w,const int _h,const int _c )
{
  UNUSED(_c);

  if ((_w==0)||(_h==0)){
    return -1;
  }

  char buf[256];
  memset(buf,'\0',256);
  sprintf( buf,
           "vidargs video=mxcfb0:dev=lcd,"
           "%ix%iM@60,if=RGB24,bpp=32 video=mxcfb1:off fbmem=8M"
           ,_w,_h );
  writeFile( "/usr/local/yq_config/lcd_config",
             buf,
             strlen(buf) );

  reboot(LINUX_REBOOT_CMD_RESTART);
  return 0;
}

int writeFile( const char* _file_name ,char *_buf,const int _size )
{
  char file_name[256];
  memset( file_name,'\0',256);
  strcpy( file_name,_file_name);

  // 以下在 aarch64 平台编译,添加的第三个参数,必须要求指定打开文件权限
  int fd = open( file_name,O_RDWR|O_CREAT|O_TRUNC,0x666);
  // 以下代码在imx6平台编译通过
  //int fd = open( file_name,O_RDWR|O_CREAT|O_TRUNC);
  if ( fd < 0 ) {
    return -1;
  }

  int ret_val = 0;
  int re_write = 0;
  do {
    // 写入内容
    lseek(fd,0,SEEK_SET);
    ssize_t ret = write(fd, _buf,_size);
    if ( ret < _size ) {
      re_write++;
      usleep(1000*re_write);
    } else {
    }
  } while ((re_write>0)&&(re_write<5));

  if ( fdatasync(fd) < 0 ) {
    usleep(5000);
    sync();
  }
  close(fd);

  return ret_val;
}
/// end of code ///
