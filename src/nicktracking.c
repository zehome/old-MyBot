#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mybot.h"
#include "my_string.h"
#include "channel.h"

/* IRC Command 353 (NAMES) */
channelNick_t *parse353 ( channelNick_t *start, const char *buffer ) 
{
  channelNick_t *work;
  int i, j;
  int bufferLen;
  char buf[64]; /* nick should not be longer than 64 bytes */
  char c;
  int type = 0;

  /* Parsing nicks */
  bufferLen = strlen(buffer);
  for (i = 0, j = 0; i < bufferLen && j < 64; i++) {
    c = buffer[i];
    switch (c) {
      case ' ':
        if (j <= 0) break;
        else {
          buf[j] = 0;

          switch (buf[0]) {
            case '@':
              type |= MODE_OP;
              break;
            case '%':
              type |= MODE_HALFOP;
              break;
            case '+':
              type |= MODE_VOICE;
              break;
            default:
              type = 0;
          }

          if (type != 0)
            work = addNick(start, type, buf+1);
          else
            work = addNick(start, type, buf);

          j = 0;
        }
        break;
      default:
        buf[j++] = c;
    }
  }
  
  /*
   * if (start)
   * printNicks (start);
   */

  return start;  
}

channelNick_t *delNick(channelNick_t *start, const char *nick)
{
  channelNick_t *work, *next, *prev;

  prev = NULL;
  work = start;
  while (work != NULL) 
  {
    next = work->next;
    if ( mystr_eq(work->nick, nick) )
    {
      if (prev == NULL)
        start = next;
      else
        prev->next = work->next;

      if (work->next != NULL)
      {
        free(work->nick);
        free(work);
      }
      break;
    }
    prev = work;
    work = next;
  }

  return start;
}

channelNick_t *addNick(channelNick_t *start, int type, const char *nick) 
{
  channelNick_t *work, *last;

  if (isOnTheChan(start, nick) == 1) {
    return start;
  }
 
  work = malloc(sizeof(channelNick_t));
  work->next = NULL;
  work->type = -1;
  work->nick = NULL;
  work->type = type;
  work->nick = (char *) calloc(strlen(nick)+1, sizeof(char));
  strncpy(work->nick, nick, strlen(nick));

  if (start == NULL)
    return work;
  
  last = start;
  while (last->next != NULL)
    last = last->next;

  last->next = work;

  return start;
}

/**
 * This function return 1 if the nick is on the channel
 * 0 *nick is not on *channel
 * 1 *nick is on the channel *channel
 *
 */
int isOnTheChan( channelNick_t *start, const char *nick)
{
  int found = 0; 

  while ((start != NULL) && !found)
  {
    if (mystr_eq(nick, start->nick) == 1)
      found = 1;
    start = start->next;
  }
  
  return found;
}

/**
 * 1 => *nick is operator
 * 0 => *nick not operator
 */
int isOperator ( channelNick_t *start, const char *nick) 
{
  return (getNickType(start, nick) & MODE_OP);
}

/**
 * This function return the status of a nick on a channel.
 *
 */
int getNickType ( channelNick_t *start, const char *nick)
{
  int type = 0;

  while ((start != NULL) && !type)
  {
    if (mystr_eq(nick, start->nick) == 1)
      type = start->type;
    
    start = start->next;
  }
  
  return type;
}

/**
 * This function print all the nick
 *
 */
void printNicks (channelNick_t *start)
{
  printf("nicks: ");
  while (start != NULL)
  {
     printf("(%s:%d) ", start->nick, start->type);
     start = start->next;
  }

  printf("\n");
}

/**
 * This function set the a new type for a nick on a channel.
 * The change is only done on the chained list (not on the irc server).
 *
 */
void setNickType(channelNick_t *start, const char *nick, int type)
{
  /* search nick/channel on the list */
  while ( (start != NULL) && (!mystr_eq(nick, start->nick)) )
    start = start->next;

  /* if nick found, set new type */
  if (start != NULL)
  {
    start->type = type;
    printf("Setting %s newtype: %d\n", start->nick, type);
  }
  else
    fprintf(stderr, "nickname %s not found.\n", nick);
}

/**
 * This function update mode, add or remove a status
 * 
 * oldmode (int)  = actual mode
 * mode    (char) = mode to change
 * type    (int)  = 0 => remove this mode
 *                  1 => add this mode
 *
 */
int updateMode(int oldmode, const char mode, int type)
{
  int ret, masque = 0;

  switch (mode) {
    case 'v': masque = MODE_VOICE; break;
    case 'h': masque = MODE_HALFOP; break;
    case 'o': masque = MODE_OP; break;
  };

  ret = (type == 0) ? oldmode & ~masque : oldmode | masque; 
      
  return ret;
}  

/* wrote by trem, Thanks trem ! */
channelNick_t *parseIRCMode( channelNick_t *start, char *str)
{
  char *mode, *nick, type = -1;
  int len, j = 0;

  len = strlen(str);

  /* get mode */
  str = my_word_next(str);
  mode = my_word_dup(str);
  str = my_word_next(str);

  /* parse mode */
  while( mode[j] != '\0' )
  {
    switch (mode[j]) {
    case '+': 
      type = 1;
      break;
    case '-':
      type = 0;
      break;
    case 'v':
    case 'h':
    case 'o':
      nick = my_word_dup(str);
      str = my_word_next(str);
      setNickType(start, nick, updateMode(getNickType(start, nick), mode[j], type));
      free(nick);
      break;
    }

    j++;
  }
  free(mode);

  return start;
}

void freeChannelNick (channelNick_t *start)
{
  channelNick_t *next;
  
  while (start != NULL)
  {
    next = start->next;
    free(start->nick);
    free(start);
    start = next;
  }
}
