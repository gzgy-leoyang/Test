#include "adapter/lcd_adapter.h"

#include <sys/reboot.h>
#include <linux/reboot.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "support/sys_info.h"
#include "support/c_debug_out.h"
#include "main.h"

#include <drm.h>
#include <drm_fourcc.h>
#include <drm_mode.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "service/udsManager.h"

int Lcd_adapter::m_piexl_x = 0;
int Lcd_adapter::m_piexl_y = 0;

int             err, drm_fd, prime_fd, retval = EXIT_FAILURE;
unsigned        int i, card;
uint32_t        fb_id, crtc_id;
drmModePlaneRes *plane_res;
drmModePlane    *plane;
drmModeFB       *fb;
char            buf[256];
uint64_t        has_dumb;
void            *buffer = nullptr;

Lcd_adapter::
Lcd_adapter() : Adapter()
{
  UdsManager* um = UdsManager::getInstance();

  UDS_operator_t op;
  op.safety   = 0;
  op.session  = 0;
  op.callback = getColor;
  um->registeGeneralOperatorWithDid(0x8800,op);

  op.callback=piexlX_rw;
  um->registeGeneralOperatorWithDid(0x8801,op);

  op.callback=piexlY_rw;
  um->registeGeneralOperatorWithDid(0x8802,op);
}

Lcd_adapter::
~Lcd_adapter()
{
  if ( buffer != nullptr ) {
    munmap(buffer, (fb->bpp>>3)*fb->width*fb->height);
    drmModeFreeFB(fb); 
    drmModeFreePlaneResources(plane_res);
    close( drm_fd );
  }
}

int Lcd_adapter::
getColor( char* _buf,int _size ) 
{
  int r,g,b=0;

  if ( buffer == nullptr ) {
    if ( init( "/dev/dri/card0") < 0 ) {
      LOGW<<" Failed to Initialize framebuffer of DRM";
    } else {
      //setPixelX(990);
      //setPixelY(540);
      if ( buffer != nullptr ) {
        getPixelColor(&r,&g,&b);
      }
    }

    if ( buffer != nullptr ) {
      munmap(buffer, (fb->bpp>>3)*fb->width*fb->height);
      drmModeFreeFB(fb); 
      drmModeFreePlaneResources(plane_res);
      close( drm_fd );
      buffer = nullptr;
    }
  }

  int sid = *(_buf+1);
  if ( sid == 0x22 ) {
    LOGV<<"Piexl..("<<m_piexl_x<<","<<m_piexl_y
        <<") Color..{"<<r<<","<<g<<","<<b<<"}";
    std::memset((_buf+4),0xAA,4);
    *(_buf+4) = 0;
    *(_buf+5) = r;
    *(_buf+6) = g;
    *(_buf+7) = b;
    return 4;
  } else if ( sid == 0x2E ) {
    //int temp = *(_buf+4);
    //temp     = ( temp << 8 ) | *(_buf+5);
    //temp     = ( temp << 8 ) | *(_buf+6);
    //temp     = ( temp << 8 ) | *(_buf+7);
    //LOGI<<" Set Touch Pressed & Released counter:"<<temp;
    //m_btn_touch_pressed = temp;
    //m_btn_touch_released= temp;
    //std::memset((_buf+4),0xAA,4);
    return 0;
  } else {
    // unknow SID
  }
}

int Lcd_adapter::
piexlX_rw( char* _buf,int _size ) 
{
  int sid = *(_buf+1);
  if ( sid == 0x22 ) {
    std::memset((_buf+4),0xAA,4);
    *(_buf+4) = (m_piexl_x & 0xFF00) >> 8 ;
    *(_buf+5) = (m_piexl_x & 0x00FF) ;
    return 2;
  } else if ( sid == 0x2E ) {
    int temp = *(_buf+4);
    temp     = ( temp << 8 ) | *(_buf+5);
    temp     = ( temp << 8 ) | *(_buf+6);
    temp     = ( temp << 8 ) | *(_buf+7);
    m_piexl_x = temp;
    LOGV<<" Set Piexl_X:"<<temp;
    std::memset((_buf+4),0xAA,4);
    return 0;
  } else {
    // unknow SID
  }
}

int Lcd_adapter::
piexlY_rw( char* _buf,int _size ) 
{
  int sid = *(_buf+1);
  if ( sid == 0x22 ) {
    std::memset((_buf+4),0xAA,4);
    *(_buf+4) = (m_piexl_y & 0xFF00) >> 8 ;
    *(_buf+5) = (m_piexl_y & 0x00FF) ;
    return 2;
  } else if ( sid == 0x2E ) {
    int temp = *(_buf+4);
    temp     = ( temp << 8 ) | *(_buf+5);
    temp     = ( temp << 8 ) | *(_buf+6);
    temp     = ( temp << 8 ) | *(_buf+7);
    m_piexl_y = temp;
    LOGV<<" Set Piexl_Y:"<<temp;
    std::memset((_buf+4),0xAA,4);
    return 0;
  } else {
    // unknow SID
  }
}

int Lcd_adapter::
init( char* _drm_dev )
{
  drm_fd = open(_drm_dev, 
                O_RDWR | O_CLOEXEC);

  if (drm_fd < 0) {
    LOGW<<" Failed to found DRM Device: "<<std::string(buf);
    return EXIT_FAILURE;
  } else {
    // 通过drmGetCap接口获取DRM能力
    // 获取buf数量,可判断当前显卡是有效的,则结束遍历
    if ( drmGetCap(drm_fd, 
                   DRM_CAP_DUMB_BUFFER, 
                   &has_dumb) >= 0 && has_dumb ) {
      // 确认显卡有效
      if (drmSetClientCap(drm_fd, 
                          DRM_CLIENT_CAP_ATOMIC, 
                          1)) {
        close( drm_fd );
        LOGW<<"EXIT_FAILURE,Failed to set drm client";
        return EXIT_FAILURE;
      }

      // 设置以下参数,则可以通过drmModeGetPlaneResources()
      // 返回返回所有支持的Plane资源,包括:cursor,overlay,primary
      if (drmSetClientCap(drm_fd, 
                          DRM_CLIENT_CAP_UNIVERSAL_PLANES, 
                          1)) {
        close( drm_fd );
        LOGW<<"EXIT_FAILURE,Failed to set drm client";
        return EXIT_FAILURE;
      }

      // 获取planes的资源，遍历所有的图层
      plane_res = drmModeGetPlaneResources(drm_fd);
      if (!plane_res) {
        close( drm_fd );
        LOGW<<"EXIT_FAILURE,Failed to get plane resource";
        return EXIT_FAILURE;
      }

       // 找到一个非0的图层立即结束循环,向下执行,获取FB
      for (i = 0; i < plane_res->count_planes; i++) {
        // 获取有关plane的信息,如果plane_id有效，则获取一个drmModePlanePtr结构，
        // 该结构包含关于指定平面的信息，例如它当前的CRTC和兼容的CRTC，
        // 当前绑定的framebuffer, gamma步数，以及平面的位置。
        plane   = drmModeGetPlane(drm_fd, 
                                  plane_res->planes[i]);
        fb_id   = plane->fb_id;
        crtc_id = plane->crtc_id;

        drmModeFreePlane(plane);
        // 找到一个"非0"的图层,退出遍历
        if (fb_id != 0 && crtc_id != 0) {
          // 获取framebuffer的资源
          // 检索有关帧缓冲缓冲区 fb_id 的信息
          fb = drmModeGetFB(drm_fd, fb_id);
          if (!fb) {
             drmModeFreePlaneResources(plane_res);
             close( drm_fd );
             LOGW<<"EXIT_FAILURE,Did NOT get framebuffer..."<<fb_id;
             return EXIT_FAILURE;
          }

          // 在DRM之上，进程获得底层framebuffer对应的fd
          // drmPrimeHandleToFD是会给dma_buf加引用计数的
          // 使用完export_dmafd后, 需要使用close(export_dmafd)来减掉引用计数
          if ( drmPrimeHandleToFD(drm_fd, 
                                   fb->handle, 
                                   O_RDONLY, 
                                   &prime_fd) < 0 ) {
            drmModeFreeFB(fb); 
            drmModeFreePlaneResources(plane_res);
            close( drm_fd );
            LOGW<<"EXIT_FAILURE,Did NOT get Handle to framebuffer";
            return EXIT_FAILURE;
          } else {
            // 建立映射关系并保持
            buffer = mmap(NULL, 
                          (fb->bpp>>3)*fb->width*fb->height,
                          PROT_READ|PROT_WRITE, 
                          MAP_PRIVATE, 
                          prime_fd, 
                          0);
            if (buffer == MAP_FAILED) {
              drmModeFreeFB(fb); 
              drmModeFreePlaneResources(plane_res);
              close( drm_fd );
              LOGW<<"EXIT_FAILURE,Failed to mmap the framebuffer";
              return EXIT_FAILURE;
            } else {
              //LOGV<<"Creating mmap...,Initilize completed";
              return 0;
            }
          }
        }
      }
    } else {
      LOGW<<"Failed to get DRM_CAP_DUMB_BUFFER or has_dumb ";
    }
  }
  return 0;
}

int Lcd_adapter::
setPixelX( const int _x)
{
  m_piexl_x = _x;
  LOGV<<"Pixel X..."<<_x;
  return 0;
}

int Lcd_adapter::
setPixelY( const int _y)
{
  m_piexl_y = _y;
  LOGV<<"Pixel Y..."<<_y;
  return 0;
}

int Lcd_adapter::
getPixelColor(int* _r,int* _g,int* _b)
{
  int localtion = m_piexl_x + m_piexl_y*fb->width; 
  int bb = (*((int*)buffer+localtion))     & 0xFF; 
  int gg = (*((int*)buffer+localtion)>>8)  & 0xFF; 
  int rr = (*((int*)buffer+localtion)>>16) & 0xFF; 
  *_r = rr;
  *_g = gg;
  *_b = bb;
  return 0;
}

int Lcd_adapter::
getPixel(const int _x,const int _y)
{
  int             err, drm_fd, prime_fd, retval = EXIT_FAILURE;
  unsigned        int i, card;
  uint32_t        fb_id, crtc_id;
  drmModePlaneRes *plane_res;
  drmModePlane    *plane;
  drmModeFB       *fb;
  char            buf[256];
  uint64_t        has_dumb;

   // 当存在多个显卡时,遍历所有显卡
  for (card = 0; ; card++) {
    snprintf(buf, sizeof(buf), "/dev/dri/card%u", card);
    drm_fd = open(buf, O_RDWR | O_CLOEXEC);
    if (drm_fd < 0) {
      LOGW<<" Failed to testing DRM Device: "<<std::string(buf);
      return EXIT_FAILURE;
    }

    // 通过drmGetCap接口获取DRM能力
    // 获取buf数量,可判断当前显卡是有效的,则结束遍历
    if (drmGetCap(drm_fd, 
                  DRM_CAP_DUMB_BUFFER, 
                  &has_dumb) >= 0 && has_dumb )
      break;
  }
  // 如果该显卡包含buf,则关闭设备
  close(drm_fd);
  LOGV<<" DRM Device is availd "<<std::string(buf);

  // 重新打开已经确认是有效的DRM设备
  drm_fd = open(buf, O_RDWR | O_CLOEXEC);
  if (drm_fd < 0) {
    LOGW<<"EXIT_FAILURE,Failed to open drm device: "<<std::string(buf);
    return EXIT_FAILURE;
  }
  LOGI<<"Opening DRM Device: "<<std::string(buf);

    // 配置DRM: DRM_CLIENT_CAP_ATOMIC,DRM_CLIENT_CAP_UNIVERSAL_PLANES
  if (drmSetClientCap(drm_fd, 
                      DRM_CLIENT_CAP_ATOMIC, 
                      1)) {
    close( drm_fd );
    LOGW<<"EXIT_FAILURE,Failed to set drm client";
    return EXIT_FAILURE;
  }
  LOGV<<"Setting DRM Device: DRM_CLIENT_CAP_ATOMIC";

  // 设置以下参数,则可以通过drmModeGetPlaneResources()
  // 返回返回所有支持的Plane资源,包括:cursor,overlay,primary
  if (drmSetClientCap(drm_fd, 
                      DRM_CLIENT_CAP_UNIVERSAL_PLANES, 
                      1)) {
    close( drm_fd );
    LOGW<<"EXIT_FAILURE,Failed to set drm client";
    return EXIT_FAILURE;
  }
  LOGV<<"Setting DRM Device: DRM_CLIENT_CAP_UNIVERSAL_PLANES";

    // 获取planes的资源，遍历所有的图层
  plane_res = drmModeGetPlaneResources(drm_fd);
  if (!plane_res) {
    close( drm_fd );
    LOGW<<"EXIT_FAILURE,Failed to get plane resource";
    return EXIT_FAILURE;
  }
  LOGV<<"Setting DRM Device: DRM_CLIENT_CAP_UNIVERSAL_PLANES";

   // 找到一个非0的图层立即结束循环,向下执行,获取FB
  for (i = 0; i < plane_res->count_planes; i++) {
    // 获取有关plane的信息,如果plane_id有效，则获取一个drmModePlanePtr结构，
    // 该结构包含关于指定平面的信息，例如它当前的CRTC和兼容的CRTC，
    // 当前绑定的framebuffer, gamma步数，以及平面的位置。
    plane   = drmModeGetPlane(drm_fd, plane_res->planes[i]);
    fb_id   = plane->fb_id;
    crtc_id = plane->crtc_id;
    LOGV<<"Plane...Fb_ID="<<fb_id<<"Crtc_ID"<<crtc_id;
    drmModeFreePlane(plane);
    // 找到一个"非0"的图层,退出遍历
    if (fb_id != 0 && crtc_id != 0)
      break;
  }
  LOGV<<"+++Plane...Fb_ID="<<fb_id<<"Crtc_ID"<<crtc_id;

   // 若最后遍历索引=图层数,则判断全部图层均不符合预期
   // 没有找到有效的图层
   if (i == plane_res->count_planes) {
      LOGW<<"EXIT_FAILURE,Did NOT found the plane";
      drmModeFreePlaneResources(plane_res);
      close( drm_fd );
      return EXIT_FAILURE;
   }

   // 获取framebuffer的资源
   // 检索有关帧缓冲缓冲区 fb_id 的信息
   fb = drmModeGetFB(drm_fd, fb_id);
   if (!fb) {
      drmModeFreePlaneResources(plane_res);
      close( drm_fd );
      LOGW<<"EXIT_FAILURE,Did NOT get framebuffer..."<<fb_id;
      return EXIT_FAILURE;
   }
   LOGV<<"Get Framebuffer from Plane...Fb_ID="<<fb_id;

   // 在DRM之上，进程获得底层framebuffer对应的fd
   // drmPrimeHandleToFD是会给dma_buf加引用计数的
   // 使用完export_dmafd后, 需要使用close(export_dmafd)来减掉引用计数
   err = drmPrimeHandleToFD(drm_fd, 
                            fb->handle, 
                            O_RDONLY, 
                            &prime_fd);
   if (err < 0) {
     drmModeFreeFB(fb); 
     drmModeFreePlaneResources(plane_res);
     close( drm_fd );
     LOGW<<"EXIT_FAILURE,Did NOT get Handle to framebuffer";
     return EXIT_FAILURE;
   }
   LOGV<<"Geting Prime Handle...";
   // 至此,已创建framebuffer并映射到用户空间
   void *buffer;
   buffer = mmap(NULL, 
                 (fb->bpp>>3)*fb->width*fb->height,
                 PROT_READ|PROT_WRITE, 
                 MAP_PRIVATE, 
                 prime_fd, 
                 0);
   if (buffer == MAP_FAILED) {
     //ret = -errno;
     //fprintf(stderr, "Unable to mmap prime buffer\n");
     //goto out_free_picture;
   }
   unsigned int len = fb->width * fb->height;
   LOGV<<"mmap Prime fd:"<<prime_fd<<" bpp:"<<fb->bpp;
   LOGV<<"Size:"<<len<<" W:"<<fb->width<<" H:"<<fb->height;

   int localtion = _x + _y*fb->width; 
   int b = *((int*)buffer+localtion) & 0xFF; 
   int g = (*((int*)buffer+localtion)>>8)  & 0xFF; 
   int r = (*((int*)buffer+localtion)>>16) & 0xFF; 
   LOGV<<" X="<<_x<<" Y="<<_y<<" Red="<<r<<" Green="<<g<<" Blue="<<b;

   munmap(buffer, (fb->bpp>>3)*fb->width*fb->height);
   LOGV<<"munmap Prime fd:"<<prime_fd;

   drmModeFreeFB(fb); 
   drmModeFreePlaneResources(plane_res);
   close( drm_fd );
   LOGV<<"Close..."<<drm_fd;
   return EXIT_SUCCESS;
}

/// end of code ///
