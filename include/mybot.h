#ifndef MYBOT_H
#define MYBOT_H

#include <sys/time.h>
#include "handlers.h"

/* Ununsed, should be removed. */
#define HOST "192.168.1.11"
#define PORT 6667
#define CHAN "#test"
#define NICK "edbot"
#define REALNAME "MyBot"

#define CONNECTION_TIMEOUT 5
#define RECONNECT_TIME 2
#define SOCK_TIMEOUT 600 /* Ten minutes */
#define PING_TRY 5

/* dummy mock */

#define VERBOSE(...) fprintf(stderr, __VA_ARGS__)
#define MESSAGE(...) fprintf(stderr, __VA_ARGS__)
#define ERROR(...)   fprintf(stderr, __VA_ARGS__)

/* Binary operators */
#define MODE_VOICE   1
#define MODE_HALFOP  2
#define MODE_OP      4

/* Error messages during config parse */
#define NICKNAME_ERROR "Erroneous config file. I can't find nick.\n"
#define CHANNEL_ERROR  "Erroneous config file. I can't find channel.\n"
#define HOSTNAME_ERROR "Erroenous config file. I cant't find hostname.\n"
#define REALNAME_ERROR "Erroenous config file. I can't find realname.\nSetting to myBot!\n"
#define PORT_ERROR     "Erroenous config file. I can't find port.\nSetting to 6667\n"
#define MASTER_NICK_ERROR "Master nick not found. Authless master impossible.\n"
#define MASTER_HOST_ERROR "Master host not found. Authless master impossible.\n"
#define MASTER_PASS_ERROR "Authed master control disabled.\n"
#define LOGFILE_ERROR     "Warning, logfile not found. Log to stdout.\n"
#define MODPATH_ERROR     "Warning, module_path not found. Setting to ./mod\n"

/* Calculs d'uptime */
time_t start_time;
struct timeval timeout;
time_t current_time;

/* for select() */
fd_set rfds;

/* The result of irc Parsing ;) */
typedef struct ircLine_t {
  char *mask;
  char *nick;
  char *user;
  char *host;
  char *command;
  char *commandArgs;
  char *value;
} ircLine_t;

void setNick(int serversocket, char *nick);

void hello(int serversocket, char *nick, char *real);

void freeIrcLine( ircLine_t *ircLine);

void initIrcLine (ircLine_t *ircLine);

void *memory_allocation(size_t size);

void pong(int socket, char *buffer);

int talkingToMe (ircLine_t *ircLine);

int isMyMasterTalking(ircLine_t *ircLine);

int isHandlerOk(ircLineHandler_t  *handler, ircLine_t *ircLine);

#endif
