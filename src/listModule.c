#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "common.h"
#include "listModule.h"

/* Demande au serveur de s'enregistrer */
myModule *init() {
  return initModule(MODULE_VERSION,  /* module version     */
                   &createHandler,   /* handlers           */
                   "listModule",     /* module Name        */
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
  
  /* Handler list channels */
  handHelper.value = "^\\.list chan.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "listChannel", mod)) != NULL)
    start = work;
  
  /* Handler list modules */
  handHelper.value = "^\\.list mod.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "listModules", mod)) != NULL)
    start = work;

  /* Handler list nicks */
  handHelper.value = "^\\.list nick.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "listNicks", mod)) != NULL)
    start = work;

  /* Handler list operators */
  handHelper.value = "^\\.list oper.*$";
  if ( (work = addHandlerHelper(start, &handHelper, "listOpers", mod)) != NULL)
    start = work;
   
  return start;
}

char *listChannel(void *args)
{
  ircLine_t *ircLine;
  modInfo_t *modInfo;
  char *retour, *dest, *buffer;
  unsigned short buflen = 256;
  int len;
  
  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;
  dest = getDestination(ircLine);

  buffer = calloc(buflen, sizeof(char));
  while (modInfo->channels != NULL)
  {
    len = strlen(modInfo->channels->channel);
    if (len+3 >= buflen)
    {
      buffer = realloc(buffer, buflen+128);
      memset(buffer+buflen, 0, 128);
      buflen += 128;
    }
    
    strncat(buffer, modInfo->channels->channel, len);
    strncat(buffer, " ", 1);
    modInfo->channels = modInfo->channels->next;
  }
  
  retour = myprintf("PRIVMSG %s :I'm on %s", dest, buffer);
  free(buffer);
  free(dest);

  return retour;
}

char *listModules(void *args)
{
  ircLine_t *ircLine;
  modInfo_t *modInfo;
  char *retour, *dest, *buffer;
  unsigned short buflen = 256;
  int len;
  
  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;
  dest = getDestination(ircLine);

  buffer = calloc(buflen, sizeof(char));
  while (modInfo->modules != NULL)
  {
    len = strlen(modInfo->modules->mod->moduleName);
    if (len+3 >= buflen)
    {
      buffer = realloc(buffer, buflen+128);
      memset(buffer+buflen, 0, 128);
      buflen += 128;
    }
    
    strncat(buffer, modInfo->modules->mod->moduleName, len);
    strncat(buffer, " ", 1);
    modInfo->modules = modInfo->modules->next;
  }
  
  retour = myprintf("PRIVMSG %s :Modules loaded: %s", dest, buffer);

  free(dest);
  free(buffer);

  return retour;
}

char *listNicks(void *args)
{
  ircLine_t *ircLine;
  modInfo_t *modInfo;
  channelNick_t *nicks;
  channelList_t *chanList;
  char *retour, *dest, *buffer;
  char *channel;
  unsigned short buflen = 256;
  int len;
  
  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;

  if ( (channel = my_word_num(ircLine->value, 3)) == NULL)
    channel = my_word_num(ircLine->commandArgs, 1);
  
  chanList = getChan(modInfo->channels, channel);
  if (chanList == NULL)
  {
    free(channel);
    return NULL;
  }

  nicks = chanList->nicks;
  if (nicks == NULL)
  {
    free(channel);
    return NULL;
  }

  buffer = calloc(buflen, sizeof(char));
  while (nicks != NULL)
  {
    len = strlen(nicks->nick);
    if (len+3 >= buflen) /* TODO: Bug here !!! */
    {
      buffer = realloc(buffer, buflen+128);
      memset(buffer+buflen, 0, 128);
      buflen += 128;
    }
    
    if (nicks->type & MODE_OP)
      strncat(buffer, "@", 1);
    else if (nicks->type & MODE_HALFOP)
      strncat(buffer, "%", 1);
    else if (nicks->type & MODE_VOICE)
      strncat(buffer, "+", 1);
    
    strncat(buffer, nicks->nick, len);
    strncat(buffer, " ", 1);
    nicks = nicks->next;
  }
  
  dest = getDestination(ircLine);
  retour = myprintf("PRIVMSG %s :Nicks on %s: %s", dest, channel, buffer);

  free(channel);  
  free(dest);
  free(buffer);

  return retour;
}

char *listOpers(void *args)
{
  ircLine_t *ircLine;
  modInfo_t *modInfo;
  char *retour, *dest, *buffer;
  unsigned short buflen = 256;
  int len, len2, len3;
  
  modInfo = (modInfo_t *) args;
  ircLine = modInfo->ircLine;

  buffer = calloc(buflen, sizeof(char));
  while (modInfo->masters != NULL)
  {
    len  = strlen(modInfo->masters->nick);
    len2 = strlen(modInfo->masters->host);
    len3 = strlen(modInfo->masters->user);
    if (len+len2+len3+4 >= buflen)
    {
      buffer = realloc(buffer, buflen+128);
      memset(buffer+buflen, 0, 128);
      buflen += 128;
    }
    
    strncat(buffer, modInfo->masters->user, len3);
    strncat(buffer, "!", 1);
    strncat(buffer, modInfo->masters->nick, len);
    strncat(buffer, "@", 1);
    strncat(buffer, modInfo->masters->host, len2);
    strncat(buffer, " ", 1);
    modInfo->masters = modInfo->masters->next;
  }
  
  if (*buffer == '\0')
    retour = NULL;
  else
  {
    dest = getDestination(ircLine);
    retour = myprintf("PRIVMSG %s :Operators: %s", dest, buffer);
    free(dest);
  }

  free(buffer);

  return retour;
}


