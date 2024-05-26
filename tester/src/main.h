#ifndef MAIN_H
#define MAIN_H

#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include <plog/Formatters/CsvFormatter.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ConsoleAppender.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Appenders/RollingFileAppender.h>

#define _DEBUG_LOG_FILE "/tmp/spy.log"
#define _ERR_LOG_FILE   "/tmp/spy.err"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#include "misc/main_data.h"
extern Main_data* main_data;
extern char config_file_name[];
#endif // MAIN_H
