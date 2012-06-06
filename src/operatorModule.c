#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "common.h"
#include "operatorModule.h"

#define TYPE_OP        1
#define TYPE_DEOP      2
#define TYPE_VOICE     3
#define TYPE_DEVOICE   4
#define TYPE_HALFOP    5
#define TYPE_DEHALFOP  6

/* Demande au serveur de s'enregistrer */
myModule *init() {
  return initModule(MODULE_VERSION,  /* module version     */
                   &createHandler,   /* handlers           */
                   MODNAME,          /* module Name        */
                   MODDESC);         /* module Description */
}

/* This function creates the handler used by the edWeb server
 * to send the job to the module.
 */
handlerList *createHandler(myModule *mod)
{
  handlerList *start, *work;
  handlerHelper_t handHelper = {
    .cmd     = "^PRIVMSG.*$",
    .value   = NULL,
    .nick    = NULL,
    .cmdArg  = NULL,
    .host    = NULL,
    .user    = NULL,
  };

  start = NULL;
  
  handHelper.value = "^\\.op.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "opCmd", mod)) != NULL)
    start = work;

  handHelper.value = "^\\.deop.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "deopCmd", mod)) != NULL)
    start = work;

  handHelper.value = "^\\.voice.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "voiceCmd", mod)) != NULL)
    start = work;
  
  handHelper.value = "^\\.devoice.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "devoiceCmd", mod)) != NULL)
    start = work;
  
  handHelper.value = "^\\.halfop.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "halfopCmd", mod)) != NULL)
    start = work;
  
  handHelper.value = "^\\.dehalfop.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "dehalfopCmd", mod)) != NULL)
    start = work;
  
  return start;
}

char *modifyNickMode(int type, void *args);

char *opCmd(void *args)       { return modifyNickMode(TYPE_OP,       args); }
char *deopCmd(void *args)     { return modifyNickMode(TYPE_DEOP,     args); }
char *voiceCmd(void *args)    { return modifyNickMode(TYPE_VOICE,    args); }
char *devoiceCmd(void *args)  { return modifyNickMode(TYPE_DEVOICE,  args); }
char *halfopCmd(void *args)   { return modifyNickMode(TYPE_HALFOP,   args); }
char *dehalfopCmd(void *args) { return modifyNickMode(TYPE_DEHALFOP, args); }

char *modifyNickMode(int type, void *args)
{
  ircLine_t *ircLine;
  modInfo_t *modInfo;
  channelNick_t *nickStatus;
  channelList_t *chanList;
  char *retour, *dest, *nick;
  char *channel;

  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;
  
  retour = NULL;

  if (! isMaster(modInfo->masters, ircLine))
  {
    fprintf(stderr, "Warn: a NON master user tried to use .op command.\n");
    fprintf(stderr, "User: %s\n", ircLine->mask);
    return NULL;
  }
  
  channel = my_word_num(ircLine->commandArgs, 1);
  
  chanList = getChan(modInfo->channels, channel);
  if (chanList == NULL)
  {
    fprintf(stderr, "Error: current channel not found.\n");
    free(channel);
    return NULL;
  }
  
  if (isOperator(chanList->nicks, modInfo->nickname) != 1)
  {
    fprintf(stderr, "Warning: I am not an operator. Exiting.\n");
    return NULL;
  }
  
  nick = getTargetNum(ircLine, 2);
  if (nick == NULL)
  {
    fprintf(stderr, "This is a bug, getTargetNum failed.\n");
  }
  else if (!isOnCurrentChannel(ircLine, modInfo, nick, &nickStatus))
  {
    printf("%s not on the chan.\n", nick);
  }
  else if (nickStatus == NULL)
  {
    fprintf(stderr, "Can't find nickStatus. This is a bug, should never happens.\n");
  } 
  else 
  {
    /* TODO: check if the bot is OP on the chan ;) */
    dest = getDestination(ircLine);
    
    switch (type)
    {
      case TYPE_OP:
        if (! (nickStatus->type & MODE_OP))
          retour = myprintf("MODE %s +o %s", dest, nick);
        break;
      case TYPE_DEOP:
        if (nickStatus->type & MODE_OP)
          retour = myprintf("MODE %s -o %s", dest, nick);
        break;
      case TYPE_VOICE:
        if (! (nickStatus->type & MODE_VOICE))
          retour = myprintf("MODE %s +v %s", dest, nick);
        break;
      case TYPE_DEVOICE:
        if (nickStatus->type & MODE_VOICE)
          retour = myprintf("MODE %s -v %s", dest, nick);
        break;
      case TYPE_HALFOP:
        if (! (nickStatus->type & MODE_HALFOP))
          retour = myprintf("MODE %s +h %s", dest, nick);
        break;
      case TYPE_DEHALFOP:
        if (nickStatus->type & MODE_HALFOP)
          retour = myprintf("MODE %s -h %s", dest, nick);
        break;
      default: fprintf(stderr, "This is a bug, the type %d is unknown.\n", type);
    }
    free(channel);
    free(dest);
  } 

  return retour;
}
