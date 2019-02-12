
#include <stdarg.h>
#include <stdio.h>
#include "dbprintf.aux.h"

int dbprintf(const char *format, ...){
  va_list args;
  va_start(args,format);  
  int ret = vfprintf(stdout, format, args);
  fflush(stdout);
  va_end(args);
  return ret;
}
