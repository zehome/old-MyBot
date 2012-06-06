#include "channel.h"
#include "nicktracking.h"
#include "my_string.h"

channelList_t *createChanListObj ()
{
  channelList_t *work;

  work = malloc(sizeof(channelList_t));
  if (work == NULL) 
  {
    fprintf(stderr, "Error creating channelList Obj. malloc failed.\n");
    exit(1);
  }

  work->next    = NULL;
  work->channel = NULL; 
  work->key     = NULL;
  work->nicks   = NULL;
  
  return work;
}

channelList_t *addChan ( channelList_t **chanList, char *channel, char *key )
{
  channelList_t *work, *last;
  int len;
  
  work = createChanListObj();

  len = strlen(channel);
  work->channel = calloc(len+1, sizeof(char));
  strncpy(work->channel, channel, len);
  
  if (key != NULL)
  {
    len = strlen(key);
    work->key = calloc(len+1, sizeof(char));
    strncpy(work->key, key, len);
  }

  /* chanList is NULL, so returns the working element as first element. */
  if (*chanList == NULL) {
    *chanList = work;
    return work;
  }
  
  /* Getting the last element in the chanList */
  last = *chanList;
  while (last->next != NULL)
    last = last->next;

  last->next = work;

  return *chanList;
}

/* Delete a chan from my chan list. */
channelList_t *delChan ( channelList_t **chanList, char *channel )
{
  channelList_t *work, *prev, *next;

  /* Searching the corresponding chan */
  prev = NULL;
  next = NULL;
  work = *chanList;
  while (work != NULL)
  {
    next = work->next;
    if (mystr_eq(work->channel, channel) == 1)
    {
      if (prev == NULL)
      {
        *chanList = next;
      } else {
        prev->next = next;
      }
      freeChannelNick(work->nicks);
      if (work->key != NULL) free(work->key);
      free(work->channel);
      free(work);
      break;
    }
    prev = work;
    work = next;
  } /* While */

  return *chanList;
}

channelList_t *getChan ( channelList_t *chanList, char *channel )
{
  while (chanList != NULL)
  {
    if (mystr_eq(chanList->channel, channel) == 1)
      return chanList;

    chanList = chanList->next;
  }

  return NULL;
}

void printChan( channelList_t *chanList )
{
  while (chanList != NULL)
  {
    printf("chan: %s\n", chanList->channel);
    chanList = chanList->next;
  }
}
