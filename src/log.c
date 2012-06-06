#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "log.h"
#include "mybot.h"
#include "my_string.h"

int logLine( FILE *logFile, ircLine_t *ircLine ) {
  char *logLine;
  char *dest;
  struct tm *curTime;
  time_t now;
  int size;
  int lineSize;
  
  if (logFile == NULL)
    return -1;

  if (ircLine->commandArgs == NULL) {
    dest = malloc(strlen(ircLine->nick)+1);
    strncpy(dest, ircLine->nick, strlen(ircLine->nick)+1);
  } else {
    if (ircLine->commandArgs[0] == '&' || ircLine->commandArgs[0] == '#')
      dest = my_word_dup(ircLine->commandArgs);
    else {
     dest = malloc(strlen(ircLine->nick)+1);
     strncpy(dest, ircLine->nick, strlen(ircLine->nick)+1);
    }
  }
  now = time(NULL);
  if (now == (time_t)-1) {
    fprintf(stderr, "Can't log line: time() failed.\n");
    perror("time");
    return -1;
  }

  curTime = malloc(sizeof(struct tm));
  localtime_r(&now, curTime); /* Get the current time */
  if (curTime == NULL) {
    fprintf(stderr, "Can't log line: localtime_r() failed.\n");
    return -1;
  }
  
  if (ircLine->value[0] == 1) /* CTCP */
    logLine = myprintf("[%.2d:%.2d:%.2d] <%s> Sends us a CTCP %s\n", curTime->tm_hour, curTime->tm_min, curTime->tm_sec, ircLine->nick, ircLine->value);
  else
    logLine = myprintf("[%.2d:%.2d:%.2d] %s <%s> %s\n", curTime->tm_hour, curTime->tm_min, curTime->tm_sec, dest, ircLine->nick, ircLine->value);
  
  free(curTime);
  free(dest);

  if (logLine != NULL)
  {
    lineSize = strlen(logLine);
    size = fwrite(logLine, sizeof(char), strlen(logLine), logFile);
    free(logLine);
  } else {
    size = 0;
  }
  
  return size;
}
