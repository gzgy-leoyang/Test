#include "page_general_setting_data.h"
#include "misc/uds_tester.h"
#include "main.h"

#include <iterator>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <string>

///////////////////////////////////////////////
Page_general_setting_data::
Page_general_setting_data(QObject *parent)
    : QObject{parent}
{
#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  std::string conf_file("Spy_default.process");
  if ( config_file_name[0] != '\0' ) {
    conf_file = std::string( config_file_name);
  }

  std::ifstream inFile(conf_file,std::ios::in);
  if (!inFile.is_open()) {
    return;
  }

  m_model = new Data_model(this);
  std::string line;
  while (std::getline(inFile, line)) {
    if ( line.find('#') == std::string::npos ) {
      std::stringstream ss(line);
      std::string item;
      std::vector<std::string> elems;

      while (std::getline(ss, item, ':')) {
        if (!item.empty()) {
          elems.push_back(item);
        }
      }
      if ( elems.size() == 3 ) {
        int    msec    = atoi(elems[0].c_str()) ;
        std::string op_name = elems[1];
        int    repeat  = atoi(elems[2].c_str()) ;
        m_model->pushData(new Data_element(QString::fromStdString( op_name ),
                                           QString("#2b2a4c"),
                                           QString("%1ms").arg(msec),
                                           QString("#2b2a4c"),
                                           this));
      }
    }
  }
  inFile.close();

  #else
  m_model = new Data_model(this);
  m_model->pushData(new Data_element(QString("step"),
                                     QString("#2b2a4c"),
                                     QString("ms"),
                                     QString("#2b2a4c"),
                                     this));
  #endif
#endif
  m_tim = new QTimer(this);
  connect(m_tim,SIGNAL(timeout()),this,SLOT(timeout_slot()));
  m_tim->start(100);
}

void Page_general_setting_data::
timeout_slot()
{

#if defined(Q_OS_WIN)
  #elif defined(Q_OS_LINUX)
  #ifdef __ARM_ARCH
  Test_sys_t* pSys = &Uds_tester::m_sys;
  static int pre_step = 0;
  if ( pre_step != uds_tester_step_index ) {
    m_model->refresh_highlight( pre_step, QString("#2b2a4c"));
    m_model->refresh_highlight( uds_tester_step_index, QString("#ea906c"));
    pre_step = uds_tester_step_index;
  }
  #else
  #endif
#endif
}

/// end of code ///
