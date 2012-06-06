#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mybot.h"
#include "parsing.h"

void parseMask (ircLine_t *ircLine) {
  char c;
  char *buf;
  int i, masklen;
  int buflen, bufpos;
  
  if (ircLine->mask == NULL) {
    printf("mask not found, can't parse mask.\n");
    return;
  }

  masklen = strlen(ircLine->mask);
  buflen = masklen +1;

  if (strstr(ircLine->mask, "!") == NULL) {
    ircLine->nick = ircLine->mask;
    return;
  }
  
  buf = calloc(1, buflen);

  for (i = 0, bufpos = 0; i < masklen; i++) {
    c = ircLine->mask[i];

    switch (c) {
      case '!':
        ircLine->nick = calloc(1, bufpos+1);
        strncpy(ircLine->nick, buf, bufpos);
        bufpos = 0;
        memset(buf, 0, buflen);
        break;
      case '@':
        ircLine->user = calloc(1, bufpos+1);
        strncpy(ircLine->user, buf, bufpos);
        bufpos = 0;
        memset(buf, 0, buflen);
        break;
      default:
        buf[bufpos++] = c;
    } /* Switch */
  }

  /* Rests: The host :D */
  if (bufpos <= 0) {
    free(buf);
    return;
  }
  
  ircLine->host = calloc(1, bufpos+1);
  strncpy(ircLine->host, buf, bufpos);
 
  free(buf);
}

ircLine_t *parseLine (char *line) {
  ircLine_t *ircLine;
  char *buffer;
  char c;
  unsigned int buflen;
  unsigned int bufpos;
  int i, k, state;
  int lineLen;

  if (line == NULL)
    return NULL;

  if (line[0] != ':') {
    return parseLineWithoutColon( line );
  }

  k = 0; /* Position */
  lineLen = strlen(line);
  /* This could be done in the main code => Optimisation */
  ircLine = malloc(sizeof(ircLine_t));
  initIrcLine (ircLine);

  buflen = 1024;
  buffer = malloc(buflen);

  memset(buffer, 0, buflen);
  /* First part */
  /* Ex: :ed!ed@zehome.com PRIVMSG #hurdfr :plop */
  /* Here we cut at ' ', so we grab the mask */
  for (i = 0, state = 0, bufpos = 0; i < lineLen; i++) {
    c = line[i];
    if (c == '\r') continue;
    if (bufpos + 2 > buflen) {
      buflen += 128;
      buffer = realloc(buffer, buflen);
    }

    if (c == ':' && state == 0) {
      state = 1;
      continue;
    }

    if (state == 1 && c != ' ') {
      buffer[bufpos++] = c;
    } else if (state == 1 && c == ' ') {
      break;
    }
  }

  /* Now I have the netmask ! */
  buffer[bufpos++] = 0;
  if (bufpos > 0) {
    ircLine->mask = calloc(1, bufpos);
    strncpy(ircLine->mask, buffer, bufpos);
  } else {
    ircLine->mask = NULL;
  }
  
  /* Skip ' ' */
  k += ++bufpos;
  
  /* Now, we grab the COMMAND sent by the server
   * It could be a command number (332 => topic),
   * or directly PRIVMSG, ...
   */
  memset(buffer, 0, buflen);
  for (i = k, bufpos = 0; i < lineLen; i++) {
    c = line[i];
    if (bufpos + 2 > buflen) {
      buflen += 128;
      buffer = realloc(buffer, buflen);
    }
    if (c == ' ') {
      break;
    }

    buffer[bufpos++] = c;
  }

  /* Now we have the command. */
  buffer[bufpos++] = 0;
  if (bufpos > 0) {
    ircLine->command = calloc(1, bufpos);
    strncpy(ircLine->command, buffer, bufpos);
  } else {
    ircLine->command = NULL;
  }
  
  k += bufpos;
  
  /* And now we are seeking for the command arguments.
   * It should stop with a colon ':'
   */
  memset(buffer, 0, buflen);
  for (i = k, bufpos = 0; i < lineLen; i++) {
    c = line[i];
    if (c == '\r') continue;
    if (c == '\n') break;
    if (bufpos + 2 > buflen) {
      buflen += 128;
      buffer = realloc(buffer, buflen);
    }

    if (c == ':') {
      break;
    }
    
    buffer[bufpos++] = c;
  }
  
  buffer[bufpos++] = 0;
  if (bufpos - 1 > 0) {
    ircLine->commandArgs = calloc(1, bufpos);
    strncpy(ircLine->commandArgs, buffer, bufpos-1);
  } else
    ircLine->commandArgs = NULL;

  k += bufpos;

  /* Now we are getting the result :-)
   *
   */

  memset(buffer, 0, buflen);
  for (i = k, bufpos = 0; i < lineLen; i++) {
    c = line[i];
    if (c == '\r') continue;

    if (bufpos + 2 > buflen) {
      buflen += 128;
      buffer = realloc(buffer, buflen);
    }

    if (c == '\n') {
      break;
    }


    buffer[bufpos++] = c;
  }

  buffer[bufpos++] = 0;
  if (bufpos - 1 > 0) {
    ircLine->value = calloc(1, bufpos);
    strncpy(ircLine->value, buffer, bufpos);
  } else {
    ircLine->value = NULL;
  }

  free(buffer);
  parseMask(ircLine);

  return ircLine;
}

ircLine_t *parseLineWithoutColon (char *line) {
  ircLine_t *ircLine;
  char *buffer;
  char c;
  int buflen, bufpos;
  int i, k;
  int lineLen;
  
  if (line == NULL) {
    return NULL;
  }

  if (line[0] == ':') {
    return parseLine( line );
  }
  
  k = 0;
  lineLen = strlen(line);
  
  ircLine = malloc(sizeof(ircLine_t));
  initIrcLine (ircLine);

  buflen = 1024;
  buffer = malloc(buflen);
  memset(buffer, 0, buflen);

  /* First part: the command */
  for (i = 0, bufpos = 0; i < lineLen; i++) {
    c = line[i];
    if (c == ' ' || c == ':')
      break;

    if (bufpos + 2 > buflen) {
      buflen += 128;
      buffer = realloc(buffer, buflen);
    }
    buffer[bufpos++] = c;
  }
  
  /* Now I have the command ! */
  buffer[bufpos++] = 0;
  if (bufpos > 0) {
    ircLine->command = calloc(1, bufpos);
    strncpy(ircLine->command, buffer, bufpos);
  } else {
    ircLine->command = NULL;
  }

  k += bufpos;

  memset(buffer, 0, buflen);
  for (i = k, bufpos = 0; i < lineLen; i++) {
    c = line[i];
    if (c == ':')
      break;

    if (bufpos + 2 > buflen) {
      buflen += 128;
      buffer = realloc(buffer, buflen);
    }
    buffer[bufpos++] = c;
  }

  buffer[bufpos++] = 0;
  if (bufpos > 0) {
    ircLine->commandArgs = calloc(1, bufpos);
    strncpy(ircLine->commandArgs, buffer, bufpos);
  } else {
    ircLine->commandArgs = NULL;
  }
  
  k += bufpos;

  memset(buffer, 0, buflen);
  for (i = k, bufpos = 0; i < lineLen; i++) {
    c = line[i];
    if (c == '\r') continue;
    if (c == '\n')
      break;

    if (bufpos + 2 > buflen) {
      buflen += 128;
      buffer = realloc(buffer, buflen);
    }
    buffer[bufpos++] = c;
   
  } 

  buffer[bufpos++] = 0;
  if (bufpos > 0) {
    ircLine->value = calloc(1, bufpos);
    strncpy(ircLine->value, buffer, bufpos);
  } else {
    ircLine->value = NULL;
  }

  free(buffer); /* thx dunky */
  return ircLine;
}
