#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "my_string.h"

void _DEBUG(int level, int line, char *file, const char *format, ...)
{
  char *z_format;
  char *debugString;
  va_list ap;
  int len, try;

  if (level > DEBUG_LEVEL)
    return;
  
  len = strlen(format) + 512; /* Try */
  z_format = malloc(len);
  if (z_format == NULL)
  {
    perror("malloc");
    exit(1);
  }

  while ( 1 )
  {
    va_start(ap, format);
    try = vsnprintf (z_format, len, format, ap);
    va_end(ap);
    if (try > -1 && try < len)
      break;
    if (try > -1)
      len = try+1;
    else
      len *= 2;
    z_format = realloc(z_format, len);
    if (z_format == NULL)
    {
      perror("realloc");
      exit(1);
    }
  }

  debugString = myprintf("[%d:%s:%d]: %s", level, file, line, z_format);
  free(z_format);
  fprintf(stderr, "%s\n", debugString);
  free(debugString);
}
