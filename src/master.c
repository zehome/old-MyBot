#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "master.h"
#include "my_string.h"

masterList_t *createMasterListObj()
{
  masterList_t *obj;

  obj = malloc(sizeof(masterList_t));
  obj->next     = NULL;
  obj->password = NULL;
  obj->host     = NULL;
  obj->user     = NULL;

  return obj;
}

masterList_t *addMaster (masterList_t **start, char *nick, char *host, char *user, char *password, int status)
{
  masterList_t *last, *master;
  int len;
  
  if (*start == NULL)
  {
    *start = createMasterListObj();
    master = *start;
  } else {
    master = createMasterListObj();
  }
  
  /* Nickname must not be NULL */
  len = strlen(nick);
  master->nick = malloc(len+1);
  strncpy(master->nick, nick, len);
  master->nick[len] = '\0';

  /* Host must not be NULL */
  len = strlen(host);
  master->host = malloc(len+1);
  strncpy(master->host, host, len);
  master->host[len] = '\0';

  /* Password must not be NULL */
  len = strlen(password);
  master->password = malloc(len+1);
  strncpy(master->password, password, len);
  master->password[len] = '\0';
  
  /* User could be null */
  if (user != NULL)
  {
    len = strlen(user);
    master->user = malloc(len+1);
    strncpy(master->user, user, len);
    master->user[len] = '\0';
  }

  master->status = status; /* Active */
  
  /* Getting the nast entry in the list */
  last = *start;
  while (last->next != NULL)
    last = last->next;
  
  if (master != *start)
    last->next = master;
  
  return *start;
}

/* 0 => no rights
 * 1 => master
 */
int isMaster(masterList_t *masters, ircLine_t *ircline)
{
  while (masters != NULL)
  {
    if ((mystr_eq(masters->nick, ircline->nick) == 1) &&
        (mystr_eq(masters->host, ircline->host) == 1) &&
        (mystr_eq(masters->user, ircline->user) == 1))
    {
      return 1;
    }
    masters = masters->next;
  }

  return 0;
}

void printMasterList ( masterList_t *start )
{
  while (start != NULL)
  {
    printf("nick: %s host: %s user: %s status: %d\n", start->nick, start->host, start->user, start->status);
    start = start->next;
  }
}
