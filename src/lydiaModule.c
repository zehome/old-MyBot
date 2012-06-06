#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "lydia.h"
#include "lydiaModule.h"
#include "common.h"

/* Demande au serveur de s'enregistrer */
myModule *init() {
  return initModule(MODULE_VERSION, &createHandler, "lydiaModule", MODDESC);
}

/* This function creates the handler used by the edWeb server
 * to send the job to the module.
 */
handlerList *createHandler(myModule *mod) 
{
  handlerList *start, *work;
  
  /* Hand Helper settings */ 
  handlerHelper_t handHelper = {
    .cmd     = "^PRIVMSG.*$",
    .value   = "^lydiaise.*$",
    .nick    = NULL,
    .cmdArg  = NULL,
    .host    = NULL,
    .user    = NULL
  };

  start = NULL;

  /* Handler lydiaise */
  if ( (work = addHandlerHelper(start, &handHelper, "lydia", mod)) != NULL)
    start = work;
  
  /* Handler verlan */
  handHelper.value   = "^verlan.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "verlan", mod)) != NULL)
    start = work;
  
  /* Handler leet sp34k */
  handHelper.value   = "^leet.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "leet", mod)) != NULL)
    start = work;

  /* Handler verif palin */
  handHelper.value   = "^verifPalin.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "verifPalin", mod)) != NULL)
    start = work;

  /* Done ?! Too easy !! */
  return start;
}

char *lydia(void *args) {
  ircLine_t *ircLine;
  char *dest, *target, *retour;
  modInfo_t *modInfo;

  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;
  
  dest = getDestination(ircLine);
  target = getTargetNum(ircLine, 2);
  
  retour = myprintf("PRIVMSG %s :%s", dest, lydiaise(target, strlen(target), 2));
  free(target);
  free(dest);

  return retour;
}

char *verlan (void *args) {
  ircLine_t *ircLine;
  char *dest, *target, *retour;
  modInfo_t *modInfo;
  
  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;
  
  dest = getDestination(ircLine);
  target = getTarget(ircLine);
  
  retour = myprintf("PRIVMSG %s :%s", dest, _verlan(target, strlen(target)));
  free(dest);

  return retour;
}

char *leet (void *args) {
  ircLine_t *ircLine;
  char *dest, *target, *retour;
  modInfo_t *modInfo;

  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;
  
  dest = getDestination(ircLine);
  target = getTarget(ircLine);
  
  retour = myprintf("PRIVMSG %s :%s", dest, _leet(target, strlen(target)));
  free(dest);

  return retour;
}

char *verifPalin (void *args) {
  ircLine_t *ircLine;
  char *dest, *target, *retour;
  modInfo_t *modInfo;

  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;

  target = my_word_num(ircLine->value, 2);
  if (target == NULL)
    return NULL;

  dest = getDestination(ircLine);
  if (_verifPalin(target, strlen(target)) == 1) {
    retour = myprintf("PRIVMSG %s :%s est un palindrome! Bravo %s!", dest, target, ircLine->nick);
  } else {
    retour = myprintf("PRIVMSG %s :%s est un petit farceur. Réessaye encore %s!", dest, ircLine->nick, ircLine->nick);
  }

  free(dest);
  free(target);

  return retour;
}
