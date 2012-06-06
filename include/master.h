#ifndef MASTER_H
#define MASTER_H

#include "mybot.h"

typedef struct masterList_t {
  char *password;
  char *host;
  char *nick;
  char *user;
  int status; /* 0 Inactive. 1 Authed and active */
  struct masterList_t *next;
} masterList_t;

masterList_t *addMaster (masterList_t **start, char *nick, char *host, char *user, char *password, int status);

int isMaster(masterList_t *masters, ircLine_t *ircline);

void printMasterList (masterList_t *start);

#endif
