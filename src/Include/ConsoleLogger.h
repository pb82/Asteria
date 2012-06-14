#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H

#include <cstdarg>
#include "Macros.h"

class ConsoleLogger {
public:
  static void info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    va_end(args);
  }

  static void error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
  }

#ifndef DEBUG
  /**
    * Redirect unwanted logs to an empty function call.
    * (Gets removed by optimizing compilers).
    */

  static void log_silent(const char *format, ...) {    
  }
#endif

#define log_info ConsoleLogger::info
#define log_error ConsoleLogger::error
// Switch all debug printouts without having ifdefs all around the code
#ifdef DEBUG
  #define log_debug log_info
#else
  #define log_debug ConsoleLogger::log_silent
#endif
};
#endif // CONSOLELOGGER_H
