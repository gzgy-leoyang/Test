#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include <QGuiApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QTranslator>
#include <QQmlContext>
#include <QDebug>
#include <QDir>
#include <QRect>
#include <QWindow>
#include <QScreen>

#include "main.h"

#include "misc/main_data.h"
#include "misc/uds_tester.h"
#include "misc/main_data.h"
#include "misc/canbus.h"

#include "page_data/page_general_info_data.h"
#include "page_data/page_general_setting_data.h"
#include "page_data/page_device_data.h"

// 版本信息
#define _AUTHOR ("yangjing@yqyd.com.cn")
#define _MAJOR  (1)
#define _MINOR  (0)


int compile_datetime( char* _cdt );
void prase_args ( int argc,char* argv[] );
void usage(void);

Main_data* main_data = nullptr;
char config_file_name[32];
int  m_debug_level    = 0;

void page_data_regeist(void)
{
  qmlRegisterType<Page_general_info_data>("qml.custom.Page_general_info_data",1,0,
                                          "General_Info_Data" );
  qmlRegisterType<Page_general_setting_data>("qml.custom.Page_general_setting_data",1,0,
                                             "General_Setting_Data" );
  qmlRegisterType<Page_device_data>("qml.custom.Page_device_data",1,0,
                                    "Device_Data");
}

int main(int argc, char *argv[])
{
  char cdt[20] = {0};
  if (  compile_datetime( cdt ) == 0) {
    char ver[64] = {0};
    snprintf( ver,50,"%i.%i.%s",
              _MAJOR,
              _MINOR,
              cdt );
    printf(" YQ_Tester %s (%s)\n",ver,_AUTHOR);
  }

  memset( config_file_name,'\0',32);
  prase_args ( argc,argv );

  // 日志的文件接口
  static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(_DEBUG_LOG_FILE,10240,3);
  // 日志的终端接口
  static plog::ColorConsoleAppender<plog::TxtFormatter> colorConsoleAppender;
  // 初始化plog,包含两个接口
  switch ( m_debug_level ) {
    case 0:
      plog::init( plog::error,&fileAppender).addAppender(&colorConsoleAppender);
      break;
    case 1:
      plog::init( plog::warning,&fileAppender).addAppender(&colorConsoleAppender);
      break;
    case 2:
      plog::init(plog::info,&fileAppender).addAppender(&colorConsoleAppender);
      break;
    case 3:
      plog::init(plog::verbose,&fileAppender).addAppender(&colorConsoleAppender);
      break;
  }
  LOGI << "YQ_Tester starting..."<<std::endl;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);

    page_data_regeist();
    QQmlApplicationEngine engine;

    // 对应main_qml页面
    main_data = new Main_data();
    engine.rootContext()->setContextProperty("Main_Data", main_data);

    const QUrl url(QStringLiteral("qrc:/page_qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}

void prase_args ( int argc,char* argv[] )
{
  if ( argv == NULL ){
    usage();
    return;
  }

  int   opt = 0;
  while ((opt =  getopt(argc, argv,"c:d:p:h")) != -1) {
    switch(opt) {
      case 'c':
        strcpy( config_file_name,optarg);
        break;
      case 'd':
        m_debug_level = atoi(optarg);
        break;
      //case 'p':
      //  strcpy(m_uds_port,optarg);
      //  break;
      case 'h':
        usage();
        exit(0);
        break;
      default:
        usage();
        exit(0);
        break;
    }
  }
}

void usage(void)
{
  printf("Usage: YQ_Tester [-h] [-c] [-d]\n");
  printf("Option:\n");
  printf(" -h Help\n");
  printf(" -c config file ,default:0\n");
  printf(" -p CAN Port used by UDS,default:can0\n");
  printf(" -d Debug level,Default:0(error) 1:warning 2:info 3:detail\n");
}

int compile_datetime( char* _cdt )
{
  if ( _cdt == NULL )
    return -1;

        char  date_buf[20] = {'\0'};
  const char* month[]      = {"Jan","Feb","Mar",
                              "Apr","May","Jun",
                              "Jul","Aug","Sep",
                              "Oct","Nov","Dec"};
  const char* _date = (char*)(__DATE__);

  //int ll = strlen(_date);
  //if ( ll < 11 ){
  //  printf(" [ERROR] The lenght of __DATE__ is loss than %i\n",ll);
  //  return -1;
  //}

  // year
  memcpy((char*)(date_buf),(char*)(_date+9), 2 );
  int mon = 0;
  for ( int i=0;i<12;i++ ){
    if ( memcmp((void*)(month[i]),(void*)(_date),3) == 0){
      mon = i + 1;
      break;
    }
  }
  // mon
  date_buf[2] = mon / 10 + '0';
  date_buf[3] = mon % 10 + '0';
  // day
  if ( __DATE__[4] == ' ' ){
    date_buf[4] = '0';
  } else {
    date_buf[4]  = __DATE__[4] ;
  }
  date_buf[5]  = __DATE__[5] ;
  // hour
  date_buf[6]  = __TIME__[0];
  date_buf[7]  = __TIME__[1];
  // min
  date_buf[8] = __TIME__[3];
  date_buf[9] = __TIME__[4];
  memcpy( _cdt,date_buf,11 );
  return 0;
}

/// end of code ///
