#ifndef MAIN_H
#define MAIN_H

#define UNUSED(x) (void)(x)

#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include <plog/Formatters/CsvFormatter.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>

#define _DEBUG_LOG_FILE "/tmp/spy.log"
#define _ERR_LOG_FILE   "/tmp/spy.err"

// 打印详细调试
#define _LONG_DEBUG_INFO (0)

#endif
