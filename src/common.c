/* This file describes a common interface for modules */

#include "common.h"

myModule *initModule(char *module_version,
                     void *registerHandler,
                     const char *moduleName,
                     const char *moduleDescr)
{
  myModule *work;
  handlerList *(*getHandlers)(myModule *mod);
  int len;

  getHandlers = registerHandler;
  
  work = malloc(sizeof(myModule));
  work->module_version = module_version;
  
  /* Copy module name */
  len = strlen(moduleName);
  work->moduleName = calloc(len, sizeof(char));
  strncpy(work->moduleName, moduleName, len);

  /* Copy module Description */
  len = strlen(moduleDescr);
  work->moduleDescription = calloc(len, sizeof(char));
  strncpy(work->moduleDescription, moduleDescr, len);

  work->md = NULL;

  /* Getting module Handlers */
  work->handlers = getHandlers(work);

  return work;
}

/* Don't forget to free! */
char *getDestination(ircLine_t *ircLine) {
  char *dest, *buf, *channel;
  int i;
  
  if ((ircLine == NULL) || (ircLine->commandArgs == NULL))
  {
    fprintf(stderr, "Error in getDestination: commandArgs or ircLine NULL :(\n");
    return NULL;
  }
  if (ircLine->commandArgs[0] == '#' || ircLine->commandArgs[0] == '&')
  {
    buf = strchr(ircLine->commandArgs, ' ');
    if (buf == NULL)
      i = strlen(ircLine->commandArgs);
    else
      i = buf - ircLine->commandArgs;

    channel = calloc(i+1, sizeof(char));
    strncpy(channel, ircLine->commandArgs, i);
    dest = channel;
  } else {
    i = strlen(ircLine->nick);
    dest = calloc(i+1, sizeof(char));
    strncpy(dest, ircLine->nick, i);
  }

  return dest;
}

char *getTarget(ircLine_t *ircLine) {
  char *target, *word;
  
  if ( ((word = my_word_next(ircLine->value)) != NULL) &&
       (*word != '\0')) {
    target = word;
  } else {
    target = ircLine->nick;
  }
  
  return target;
}

/* Don't forget to free! */
char *getTargetNum(ircLine_t *ircLine, int num) {
  char *word, *target;
  int len;
  
  if ( (word = my_word_num(ircLine->value, 2)) != NULL) {
    target = word;
  } else {
    len = strlen(ircLine->nick);
    target = calloc(len+1, sizeof(char));
    strncpy(target, ircLine->nick, len);
  }
 
  return target;
}

int compileRegex(char *regex, regex_t *dest) {
  int regReturn;
  char regError[REGERROR_SIZE];

  regReturn = regcomp(dest, regex, 0);
  if (regReturn != 0)
  {
    regerror(regReturn, dest, regError, REGERROR_SIZE);
    fprintf(stderr, "regexp compilation error: %s\n", regError);
    return 1;
  }
  
  return 0;
}

handlerList *addHandlerHelper(handlerList *start, handlerHelper_t *h, char *func, void *mod) {
  ircLineHandler_t *ircLine;
  handlerList *handler;
  handlerList *retour;
  int ok = 0;
  
  ircLine = malloc(sizeof(ircLineHandler_t));
  
  if (h->cmd != NULL) {
    ok = compileRegex(h->cmd, &ircLine->command);
    ircLine->useCommand = 1;
  } else
    ircLine->useCommand = 0;
  
  if ((h->cmdArg != NULL) && (ok == 0)) {
    ok = compileRegex(h->cmdArg, &ircLine->commandArgs);
    ircLine->useCommandArgs = 1;
  } else
    ircLine->useCommandArgs = 0;

  if ((h->host != NULL) && (ok == 0)) {
    ok = compileRegex(h->host, &ircLine->host);
    ircLine->useHost = 1;
  } else
    ircLine->useHost = 0;
  
  if ((h->user != NULL) && (ok == 0)) {
    ok = compileRegex(h->user, &ircLine->user);
    ircLine->useUser = 1;
  } else
    ircLine->useUser = 0;
  
  if ((h->nick != NULL) && (ok == 0)) {
    ok = compileRegex(h->nick, &ircLine->nick);
    ircLine->useNick = 1;
  } else
    ircLine->useNick = 0;
  
  if ((h->value != NULL) && (ok == 0)) {
    ok = compileRegex(h->value, &ircLine->value);
    ircLine->useValue = 1;
  } else
    ircLine->useValue = 0;
  
  
  if (ok == 0) {
    handler = malloc(sizeof(handlerList));
    handler->function = func;
    handler->module   = mod;
    handler->state    = 1;
    handler->handler  = ircLine;

    retour = addModHandler(start, handler);
    free(handler);
    return retour;
  } else {
    free(ircLine);
    return NULL;
  }
}

/** 
 * This function returns True if the nick is on the channel
 * of the ircLine. If it's Private Message, the function
 * returns False.
 * If the nick is unknown, it will returns False.
 */
int isOnCurrentChannel(ircLine_t *ircLine, modInfo_t *modInfo, char *nick, channelNick_t **target)
{
  char *channel;
  channelList_t *chanList;
  channelNick_t *nicks;

  *target = NULL; /* I know I override that value just after */
  
  channel = getDestination(ircLine);
  /* We don't know here if it's a nick or a chan */
  /* The next check will say that ;) */
  
  chanList = getChan(modInfo->channels, channel);
  if (chanList == NULL)
  {
    printf("chanList null\n");
    free(channel);
    return 0;
  }

  free(channel);

  nicks = chanList->nicks;
  /* Should not happens */
  if (nicks == NULL)
  {
    printf("nicks null\n");
    return 0;
  }

  while (nicks != NULL)
  {
    printf("comparing %s with %s\n", nicks->nick, nick);
    if (mystr_eq(nicks->nick, nick) == 1)
    {
      *target = nicks;
      return 1;
    }
    nicks = nicks->next;
  }

  return 0;
}
