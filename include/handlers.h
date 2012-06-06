#ifndef HANDLER_H
#define HANDLER_H

#include <regex.h>

typedef struct ircLineHandler_t {
  regex_t nick;
  unsigned short useNick;
  regex_t host;
  unsigned short useHost;
  regex_t user;
  unsigned short useUser;
  regex_t command;
  unsigned short useCommand;
  regex_t commandArgs;
  unsigned short useCommandArgs;
  regex_t value;
  unsigned short useValue;
} ircLineHandler_t;

typedef struct handlerList {
  struct handlerList *next;
  char *function;
  void *module;
  int state;
  ircLineHandler_t *handler;
} handlerList;

handlerList *createHandlerListObj();
handlerList *addModHandler(handlerList *start, handlerList *handler);
handlerList *removeHandlerListElement(handlerList *list, handlerList *element);

int canAddHandler(handlerList *globalHandList, handlerList *handler);

#endif
