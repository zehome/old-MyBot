#ifndef COMMON_MOD_H
#define COMMON_MOD_H

#include "handlers.h"
#include "modules.h"
#include "channel.h"
#include "mybot.h"
#include "my_string.h"
#include "master.h"

/* This function is used to initialize a module. It's called directly
 * from the module host.
 */
myModule *initModule(
                     char *module_version,        /* Module version       */
                     void *registerHandler,       /* Handlers             */
                     const char *moduleName,      /* module Name          */
                     const char *moduleDescr      /* Module description   */
                    );

typedef struct handlerHelper_t {
  char *cmd;
  char *cmdArg;
  char *nick;
  char *user;
  char *host;
  char *value;
} handlerHelper_t;

/* This struct is sent by mybot to modules. */
typedef struct modInfo_t {
  channelList_t *channels;
  masterList_t  *masters;
  ircLine_t     *ircLine;
  moduleList    *modules;
  handlerList   *handlers;
  char *nickname; /* robot nickname */
} modInfo_t;

int compileRegex(char *regex, regex_t *dest);


char *getTargetNum(ircLine_t *ircLine, int num);
char *getTarget(ircLine_t *ircLine);
char *getDestination(ircLine_t *ircLine);

/* The most powerfull!! */
handlerList *addHandlerHelper(handlerList *start, handlerHelper_t *h, char *func, void *mod);

int isOnCurrentChannel(ircLine_t *ircLine, modInfo_t *modInfo, char *nick, channelNick_t **target);
#define REGERROR_SIZE 256

#endif
