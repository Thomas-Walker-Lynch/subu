
#include "debug.lib.h"

#include <stdarg.h>
#include <stdio.h>

int debug_printf(const char *format, ...){
  va_list args;
  va_start(args,format);  
  int ret = vfprintf(stdout, format, args);
  fflush(stdout);
  va_end(args);
  return ret;
}
