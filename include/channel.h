#ifndef CHANNEL_H
#define CHANNEL_H

#include "nicktracking.h"

typedef struct channelList_t {
  struct channelList_t *next;

  char *channel;        /* Chan name            */
  char *key;            /* If the chan is +k    */
  channelNick_t *nicks; /* Users on the channel */
} channelList_t;


channelList_t *createChanListObj ();
channelList_t *addChan ( channelList_t **chanList, char *channel, char *key );
channelList_t *delChan ( channelList_t **chanList, char *channel );
channelList_t *getChan ( channelList_t  *chanList, char *channel );

void printChan( channelList_t *chanList );

#endif
