#ifndef DEBUG_OUT_H
#define DEBUG_OUT_H

#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include <plog/Formatters/CsvFormatter.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>

//#define _DEBUG (1)
#define _DEBUG_LOG_FILE "/tmp/spy.log"
#define _ERR_LOG_FILE   "/tmp/spy.err"



#endif
