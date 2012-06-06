#ifndef NICKTRACKING_H
#define NICKTRACKING_H

/* Type: 
 *    0 No particuliar rights
 *    1 Operator
 *    2 Half Operator
 *    3 Voiced
 */
typedef struct channelNick_t {
  char *chan;
  char *nick;
  int type;
  struct channelNick_t *next;
} channelNick_t;

channelNick_t *parse353 ( channelNick_t *start, const char *buffer );
channelNick_t *delNick(channelNick_t *start, const char *nick);
channelNick_t *addNick(channelNick_t *start, int type, const char *nick);
channelNick_t *parseIRCMode( channelNick_t *start, char *str);

int isOnTheChan( channelNick_t *start, const char *nick);
int isOperator ( channelNick_t *start, const char *nick);
int getNickType ( channelNick_t *start, const char *nick);
int updateMode(int oldmode, const char mode, int type);

void printNicks (channelNick_t *start);
void setNickType(channelNick_t *start, const char *nick, int type);
void freeChannelNick (channelNick_t *start);

#endif
