#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "common.h"
#include "pingModule.h"

/* Demande au serveur de s'enregistrer */
myModule *init() {
  return initModule(MODULE_VERSION,  /* module version     */
                   &createHandler,   /* handlers           */
                   "pingModule",     /* module Name        */
                   MODDESC);         /* module Description */
}

handlerList *createHandler(myModule *mod)
{
  handlerHelper_t handHelper = {
    .cmd     = "^PRIVMSG.*$",
    .value   = "^ping.*$",
    .nick    = NULL,
    .cmdArg  = NULL,
    .host    = NULL,
    .user    = NULL,
  };

  return addHandlerHelper(NULL, &handHelper, "ping_pong", mod);
}

char *ping_pong(void *args) {
  ircLine_t *ircLine;
  char *retour, *target, *dest;
  modInfo_t *modInfo;

  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;

  dest = getDestination(ircLine);
  target = getTargetNum(ircLine, 2);
  
  retour = myprintf("PRIVMSG %s :%s: pong!", dest, target);
  free(target);
  free(dest);

  return retour;
}
