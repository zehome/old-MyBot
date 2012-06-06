/*** This code is under GNU/GPL Licence (v2)
 *
 *  Written by Laurent Coustet <ed@zehome.com>
 *
 *  & Trem <do.not@spam.me>
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>

#include "mybot.h"
#include "channel.h"
#include "nicktracking.h"
#include "master.h"
#include "net.h"
#include "parsing.h"
#include "config.h"
#include "my_string.h"
#include "modules.h"
#include "log.h"
#include "debug.h"
#include "common.h"

/* Some global vars commonly used */
/* IRC specific stuff */
char *nickname, *realname;
struct {
  char *nick;
  char *host;
  char *password;
} master;
masterList_t  *masters = NULL; 

/* Server connection stuff */
char *hostname, *serverport;

/* Configuration */
char *configFile;
  
/* Usefull for debugging :p */
void *memory_allocation(size_t size) {
  void *ptr;
  if ((ptr = calloc(size, sizeof(char))) == NULL) {
    perror("Error allocating memory. Out Of Memory ?\n");
    exit(-1);
  }
  return ptr;
}

int main (int argc, char **argv) 
{
  int socket, retval;
  int netinfo_socket;
  int client_socket;
  int fdmax = 0;
  struct sockaddr_storage client_addr;
  socklen_t addrlen;
  char *buffer, *modBuffer, *channel;
  char *modPath, *line, *newnick;
  char *Module_Path, *logPath;
  char *key;
  char *network_port;
  char c;
  int size = 0, i = 0, pingTry = 0;
  FILE *logFile;

  config_t      *config, *workconf;
  ircLine_t     *ircLine;
  channelNick_t *chanNicks;
  channelList_t *chanList = NULL, *chanListWork = NULL;
  
  /* Module needs ifdef ? */
  moduleList  *modList   = NULL;
  handlerList *handlers  = NULL;
  myModule    *module    = NULL;
  myModule    *useModule = NULL;
  handlerList *workHdrList;
  modInfo_t   *modInfo   = NULL;
  
  addrlen = sizeof(client_addr);
  nickname = NULL;
  modInfo = (modInfo_t *) malloc(sizeof(modInfo_t));

  printf("IRCBot Starting\n");

  if (argc < 2)
    configFile = "./mybot.conf";
  else
    configFile = argv[1];

  printf("Reading config from from %s\n", configFile);   
  config = parseConfig ( configFile );
  
  if (config == NULL) 
  {
    fprintf(stderr, "Could not read the config file.\n");
    exit(1);
  }

  /* Usefull debug ;) */
  _conf_printConfig(config);
  
  if (argc < 2) 
  {
    set_str_from_conf(config, "nick",          &nickname,    NULL,     NICKNAME_ERROR,    1);
  } else {
    nickname = memory_allocation(strlen(argv[2])+1);
    strncpy(nickname, argv[2], strlen(argv[2]));
  }
  
  set_str_from_conf(config, "hostname",        &hostname,    NULL,     HOSTNAME_ERROR,    1);
  set_str_from_conf(config, "port",            &serverport, "6667",    PORT_ERROR,        0);
  set_str_from_conf(config, "realname",        &realname,   "myBot",   REALNAME_ERROR,    0);
  set_str_from_conf(config, "master_nick",     &master.nick, NULL,     MASTER_NICK_ERROR, 0);
  set_str_from_conf(config, "master_host",     &master.host, NULL,     MASTER_HOST_ERROR, 0);
  set_str_from_conf(config, "master_password", &master.password, NULL, MASTER_PASS_ERROR, 0);
  set_str_from_conf(config, "logFile",         &logPath,      NULL,    LOGFILE_ERROR,     0);
  set_str_from_conf(config, "module_path",     &Module_Path, "./mod",  MODPATH_ERROR,     0);
  set_str_from_conf(config, "networkport",     &network_port, NULL,    NULL,              0);
  
  workconf = config;
  /* Channel registering */
  while (workconf != NULL)
  {
    workconf = _conf_getValue(workconf, "channel", &buffer);
    if (buffer == NULL)
      break;
    
    channel = my_word_num(buffer, 1);
    key = my_word_num(buffer, 2);
    addChan(&chanList, channel, key);
    free(channel);
    free(key);
    free(buffer);
  }

  if (logPath != NULL) 
  {
    logFile = fopen(logPath, "a+");
    free(logPath);
  } else {
    logFile = stdout;
  }

  /* Modules */
  workconf = config;
  modPath = NULL;
  while (workconf != NULL)
  {
    workconf = _conf_getValue(workconf, "module", &modPath);
    if (modPath == NULL)
      break;

    module = registerModule(&handlers, &modList, Module_Path, modPath);
    if (module == NULL) 
    {
      fprintf(stderr, "Failed to load module %s\n", modPath);
      exit(1);
    }
  }
  
  if (network_port != NULL)
  {
    netinfo_socket = server_listen(NULL, (const char *)network_port, AF_INET, SOCK_STREAM);
    if (netinfo_socket < 0)
    {
      (void) perror("server_listen");
      netinfo_socket = -1;
    } else {
      int ok;
      if ((setsockopt(netinfo_socket, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(netinfo_socket))) == -1)
      {
        perror("setsockopt");
        netinfo_socket = -1;
      } else {
        DEBUG("netinfo Listening on port %s.", network_port);
      }
    }
  }
  
  line = memory_allocation(1024);
  socket = server_connect(hostname, serverport);
  if (socket <= 0) 
  {
    (void) D_ERROR("Can't establish the connection.");
    exit(2);
  }
  
  chanNicks = malloc(sizeof(channelNick_t));
  chanNicks->nick = NULL;
  chanNicks->type = -1;
  chanNicks->chan = NULL;
  chanNicks->next = NULL;

  (void) hello(socket, nickname, realname);

  while(i < 1024) 
  {
    size = 0;
    fdmax = 0;
    timeout.tv_sec = SOCK_TIMEOUT;
    timeout.tv_usec = 0;
    if (netinfo_socket > 0)
      fdmax += netinfo_socket;
    fdmax += socket+1;

    FD_ZERO(&rfds);
    FD_SET(socket, &rfds);
    if (netinfo_socket > 0)
      FD_SET(netinfo_socket, &rfds);

    retval = select(fdmax, &rfds, NULL, NULL, &timeout);
    if (FD_ISSET(socket, &rfds))
    {
      if (retval <= 0)
      {
        for (pingTry = 0; pingTry < PING_TRY; pingTry++)
        {
          /* socket timeout */
          /* Try to ping the server */
          /* The rfds was cleared by the precedent select! */
          FD_ZERO(&rfds);
          FD_SET(socket, &rfds);
        
          (void) mySend(socket, "PING %s\n", hostname);
          timeout.tv_sec = 20;
          timeout.tv_usec = 0;
          retval = select(socket+1, &rfds, NULL, NULL, &timeout);
          if (FD_ISSET(socket, &rfds))
            break;
        }
        if (pingTry >= PING_TRY)
        {
          (void) close(socket);
          socket = -1;
          while (socket <= 0)
          {
            (void) DEBUG("Get disconnected from the server... Trying to reconnect");
            (void) sleep(5);
            socket = server_connect(hostname, serverport);
            (void) hello(socket, nickname, realname);
          }
        }
      }
    } else if (FD_ISSET(netinfo_socket, &rfds)) {
      if ((client_socket = accept(netinfo_socket, (struct sockaddr *) &client_addr, &addrlen)) == -1)
      {
        (void) perror("accept");
        client_socket = -1;
      }
      (void) DEBUG("Received connexion for netinfo.");
      /* Proto:
       * COMMAND channel\n
       *
       * Available commands: 
       *  - HOWMANY channel
       *  - NICKS   channel Not Implemented
       *  - TOPIC   channel Not Implemented
       *
       *  Return values:
       *  HOWMANY -> number of nicks\n
       *  NICKS   -> nick1 nick2 nick3 nick4\n
       *  TOPIC   -> Topic\n
       *  
       *  or Unknown -> ERROR [description]\n
       *  (in [] ar marqued optional parameters.)
       */
      {
        char rcv_buffer[256];
        char *ni_cmd, *ni_chan;
        char c;
        int pos = 0;
        int ok = 0;
        channelList_t *ni_chanList;
        channelNick_t *ni_nickList;
        int nbnicks;

        (void) memset(&rcv_buffer, 0, 256); /* Useless */

        while (read(client_socket, &c, 1) > 0)
        {
          if (pos >= 256)  break;

          if (c == '\n')
          {
            ok = 1;
            break;
          }
          rcv_buffer[pos++] = c;
        }
        rcv_buffer[pos] = 0;

        if (ok == 1)
        {
          ni_cmd  = my_word_num((char *) &rcv_buffer, 1);
          if (ni_cmd == NULL)
          {
            DEBUG("Invalid query: command not found.");
            goto endni;
          }
          ni_chan = my_word_num((char *) &rcv_buffer, 2);
          if (ni_chan == NULL)
          {
            DEBUG("Invalid query: channel not found.");
            (void) free(ni_cmd);
            goto endni;
          }
          ni_chan = strip_crlf( ni_chan );

          /* Command HowMany */
          if (mystr_eq(ni_cmd, "HOWMANY") == 1)
          {
            ni_chanList = getChan(chanList, ni_chan);
            if (ni_chanList == NULL)
            {
              mySend(client_socket, "ERROR unknown channel %s\n", ni_chan);
              goto endni;
            }
            ni_nickList = ni_chanList->nicks;
            nbnicks = 0;
            while (ni_nickList != NULL)
            {
              ni_nickList = ni_nickList->next;
              nbnicks++;
            }
            
            mySend(client_socket, "%s %d\n", ni_chan, nbnicks);
          }

          /* Command NICKS */
          else if (mystr_eq(ni_cmd, "NICKS") == 1) 
          {
            char *nick_buffer;
            unsigned int nick_buflen = 256, nick_bufpos = 0;
            int nick_len;

            ni_chanList = getChan(chanList, ni_chan);
            if (ni_chanList == NULL)
            {
              mySend(client_socket, "ERROR unknown channel %s\n", ni_chan);
              goto endni;
            }
            
            ni_nickList = ni_chanList->nicks;

            nick_buffer = (char *) calloc(nick_buflen, sizeof(char));
            while (ni_nickList != NULL)
            {
              nick_len = strlen(ni_nickList->nick);
              if (nick_bufpos+nick_len+3 >= nick_buflen)
              {
                nick_buflen += 128;
                nick_buffer = (char *) realloc((void *) nick_buffer, nick_buflen);
              }

              strncat(nick_buffer, ni_nickList->nick, nick_len);
              strncat(nick_buffer, " ", 1);
              nick_bufpos += nick_len+1;
              ni_nickList = ni_nickList->next;
            }
            
            mySend(client_socket, "%s %s\n", ni_chan, nick_buffer);
            (void) free(nick_buffer);
          /* Unknown Command */
          } else {
            DEBUG("Command %s not implemented.", ni_cmd);
          }
          
          (void) free(ni_cmd);
          (void) free(ni_chan);
        } else {
          DEBUG("Invalid command received from the client.\n");
        }
        
      }
      endni:
      (void) close(client_socket);
    } else {
      (void) DEBUG("Unknown event with select... Discarding.");
    }

    if (! FD_ISSET(socket, &rfds))
      continue;
    
    size = recv(socket, &c, 1, 0);
    if (size <= 0) { 
      (void) D_ERROR("Error: received 0 bytes.\n");
      (void) DEBUG("Get disconnected from the server... Trying to reconnect");
      (void) sleep(5);
      socket = server_connect(hostname, serverport);
      (void) hello(socket, nickname, realname);
    }

    if (c != '\n') 
    {
      line[i++] = c;
      continue;
    } else {
      line[i] = 0;
      i = 0;
    }

    if (line == NULL) 
      continue;
    ircLine = parseLine(line);
    if (ircLine == NULL)
      continue;
    
    if (strncmp(ircLine->command, "PRIVMSG", 7) == 0) 
    {
      logLine (logFile, ircLine);
      /* This is a CTCP */
      if ((ircLine->value[0] == 1))
      {
        
        /* ctcp ping */
        if (strncmp(ircLine->value+1, "PING", 4) == 0)
        {
          buffer = my_word_next(ircLine->value);
          /* The \001 is in the buffer! */
          if (buffer != NULL)
            mySend(socket, "NOTICE %s :\001PING %s\n", ircLine->nick, buffer);
        } 
        
        /* ctcp version */
        else if (strncmp(ircLine->value+1, "VERSION", 7) == 0)
        {
          mySend(socket, "NOTICE %s :\001VERSION myBot %s\001\n", ircLine->nick, "0.1"); /* TODO: use constant. */
        }

        /* ctcp source */
        /* Ugly anwser, ugly protocol, ugly ctcp. */
        else if (strncmp(ircLine->value+1, "SOURCE", 6) == 0)
        {
          mySend(socket, "NOTICE %s :\001SOURCE http://ed.zehome.com/?page=mybot\001\n", ircLine->nick);
        }
      }
      
      /* .load */
      else if ((strncmp(ircLine->value, ".load", 5) == 0) &&
          (isMyMasterTalking(ircLine) == 1))
      {
        buffer = my_word_num(ircLine->value, 2);
        if ((buffer != NULL) && (*buffer != 0)) 
        {
          DEBUG("Loading: %s", buffer);
          module = registerModule(&handlers, &modList, Module_Path, buffer);
          if (module == NULL) 
          {
            DEBUG("Failed to load module %s", modPath);
            mySend(socket, "NOTICE %s :Unable to load %s.\n", ircLine->nick, buffer);
          }
        }
        free(buffer);
      }
      
      /* .unload */
      else if ((strncmp(ircLine->value, ".unload", 7) == 0) &&
               (isMyMasterTalking(ircLine) == 1))
      {
        buffer = my_word_num(ircLine->value, 2);
        if ((buffer != NULL) && (*buffer != 0)) 
        {
          DEBUG("unloading: %s", buffer);
          module = getModuleByName(modList, buffer);
          if (module == NULL)
            DEBUG("Can't find module %s.", buffer);
          else
            unloadModule(&handlers, &modList, module);
        }
      }
      
      /* .join */
      else if ((strncmp(ircLine->value, ".join", 5) == 0) &&
               (isMyMasterTalking(ircLine) == 1))
      {
        channel = my_word_num(ircLine->value, 2);
        key = my_word_num(ircLine->value, 3);
        if (channel == NULL)
        {
          mySend(socket, "PRIVMSG %s :Please specify a channel to join.\n", ircLine->nick);
        } else {
          if (getChan (chanList, channel) == NULL)
          {
            addChan(&chanList, channel, key);
            if (key != NULL)
            {
              mySend(socket, "JOIN %s %s\n", channel, key);
              free(key);
            } else
              mySend(socket, "JOIN %s\n", channel);
          } else {
            mySend(socket, "NOTICE %s :I'm already on %s dude :)\n", ircLine->nick, channel);
          }
          free(channel);
        }
      }
      
      /* .part */
      else if ((strncmp(ircLine->value, ".part", 5) == 0) &&
               (isMyMasterTalking(ircLine) == 1))
      {
        buffer = my_word_num(ircLine->value, 2);
        if ((buffer != NULL) && ((*buffer == '&') || (*buffer = '#')))
        {
          if (getChan(chanList, buffer) != NULL)
          {
            mySend(socket, "PART %s\n", buffer);
            delChan(&chanList, buffer);
          }
        } else {
          channel = my_word_num(ircLine->commandArgs, 1);
          if ((channel != NULL) && ((*channel == '&') || (*channel == '#')))
          {
            if (getChan(chanList, channel) != NULL)
            {
              mySend(socket, "PART %s\n", channel);
              delChan(&chanList, channel);
              free(channel);
            }
          }
        } 
      }
      
      /* .quit */
      else if ((strncmp(ircLine->value, ".quit", 5) == 0) &&
               (isMyMasterTalking(ircLine) == 1))
      {
        mySend(socket, "QUIT :Good bye!\n");
        DEBUG("Exiting due to .quit.");
        exit(0);
      }
      
      /* .auth */
      else if ((strncmp(ircLine->value, ".auth", 5) == 0) &&
              (talkingToMe(ircLine) == 1))
      {
        if ((strlen(ircLine->value) > 5) && 
            (mystr_eq(ircLine->value+6, master.password) == 1))
        {
          if (isMaster(masters, ircLine) == 1)
          {
            mySend(socket, "PRIVMSG %s :You already are my master dude!\n", ircLine->nick);
          } else {
            addMaster (&masters, ircLine->nick, ircLine->host, ircLine->user, master.password, 1);
            mySend(socket, "PRIVMSG %s :You are now one of my masters.\n", ircLine->nick);
          }
        } else {
          mySend(socket, "PRIVMSG %s :Incorrect password.\n", ircLine->nick);
        }
      }
    } else if (strncmp(ircLine->command, "JOIN", 4) == 0 ) {
      buffer       = my_word_num(ircLine->value, 1);
      chanListWork = getChan (chanList, buffer);
      if (chanListWork != NULL)
        chanListWork->nicks = addNick(chanListWork->nicks, 0, ircLine->nick);
      free(buffer);
    } else if (strncmp(ircLine->command, "PART", 4) == 0) {
      buffer       = my_word_num(ircLine->commandArgs, 1);
      chanListWork = getChan (chanList, buffer);
      if (chanListWork != NULL)
        chanListWork->nicks = delNick(chanListWork->nicks, ircLine->nick);
      free(buffer);
    } else if (strncmp(ircLine->command, "QUIT", 4) == 0) {
      while (chanListWork != NULL)
      {
        chanListWork->nicks = delNick(chanListWork->nicks, ircLine->nick);
        chanListWork = chanListWork->next;
      }
    } else if (strncmp(ircLine->command, "MODE", 4) == 0 ) {
      buffer       = my_word_num(ircLine->commandArgs, 1);
      if ((buffer != NULL) && (*buffer != '#' || *buffer != '&'))
      {
        chanListWork = getChan (chanList, buffer);
        if (chanListWork != NULL)
          chanListWork->nicks = parseIRCMode(chanListWork->nicks, ircLine->commandArgs);
        free(buffer);
      }
    } else if (strncmp(ircLine->command, "PING", 4) == 0 ) {
      pong(socket, line);
    } else if (strncmp(ircLine->command, "ERROR", 5) == 0 ) {
      fprintf(stderr, "Server ERROR: %s\n", ircLine->value);
      exit(-1);
    } else if ( (strncmp(ircLine->command, "433", 3) == 0) ||
                (strncmp(ircLine->command, "462", 3) == 0))
    {
      /* Nickname already in use. */
      newnick = myprintf("%s_", nickname);
      mySend(socket, "NICK %s_\n", newnick);
      free(nickname);
      nickname = newnick;
    } else if (strncmp(ircLine->command, "372", 3) == 0) {
      /* MOTD message */
      printf("MOTD: %s\n", ircLine->value);
    } else if (strncmp(ircLine->command, "353", 3) == 0) {
      buffer = my_word_num(ircLine->commandArgs, 3);
      chanListWork = getChan (chanList, buffer);
      if (chanListWork != NULL)
        parse353(chanListWork->nicks, ircLine->value);
      free(buffer);
    } else if (strncmp(ircLine->command, "001", 3) == 0) {
      /* We can safely join channels .... */
      chanListWork = chanList;
      while (chanListWork != NULL)
      {
        if (chanListWork->key != NULL)
          mySend(socket, "JOIN %s %s\n", chanListWork->channel, chanListWork->key);
        else
          mySend(socket, "JOIN %s\n", chanListWork->channel);
        chanListWork = chanListWork->next;
      }
    } else if (strncmp(ircLine->command, "475", 3) == 0) {
      channel = my_word_num(ircLine->commandArgs, 2);
      delChan(&chanList, channel);
      printf("Can't join channel %s: invalid key specified.\n", channel);
      free(channel);
    }
    /* DEBUG("Trying a module..."); */
    workHdrList = handlers;
    while (workHdrList != NULL) 
    {
      if (isHandlerOk(workHdrList->handler, ircLine) == 1)
      {
        modInfo->modules  = modList;
        modInfo->handlers = handlers;
        modInfo->ircLine  = ircLine;
        modInfo->channels = chanList;
        modInfo->masters  = masters;
        modInfo->nickname = nickname;

        useModule = (myModule *) workHdrList->module;
        DEBUG("Executing module `%s'.", useModule->moduleName);
        modBuffer = (char *) execFunction(useModule->md, workHdrList->function, (void *)modInfo);
        if (modBuffer != NULL)
          mySend(socket, "%s\n", modBuffer);
        else
          DEBUG("Module executed, but returns nothing.");
      }
      workHdrList = workHdrList->next;
    }

    freeIrcLine(ircLine);
    free (ircLine);
  }
  
  /* Cleanup */
  free(modInfo);
  return 0;
}

void pong(int socket, char *buffer) {
  if (! buffer) 
    return;

  while (*buffer != ':' && *buffer != 0) buffer++;
  buffer++;
  mySend(socket, "PONG %s\n", buffer);

  return;
}

/* initialize the structure */
void initIrcLine (ircLine_t *ircLine) 
{
  ircLine->mask        = NULL;
  ircLine->nick        = NULL;
  ircLine->user        = NULL;
  ircLine->host        = NULL;
  ircLine->command     = NULL;
  ircLine->commandArgs = NULL;
  ircLine->value       = NULL;
}

/* Returns 1 if the ircLine is addressed to me. */
int talkingToMe (ircLine_t *ircLine) 
{
  char *word;
  int retval = 0;
  
  word = my_word_num(ircLine->commandArgs, 1);
  if (word != NULL)
  {
    retval = mystr_eq(word, nickname);
    free(word);
  }
  
  return retval;
}

/* If someone is considered as the master of the bot,
 * this function returns 1
 * else it will returns 0.
 */
int isMyMasterTalking(ircLine_t *ircLine)
{

  /* First, try nick / host combinaison */
  if( mystr_eq(master.nick, ircLine->nick) && mystr_eq(master.host, ircLine->host) )
    return 1;
  
  /* Try authed by password users... */
  if ((masters != NULL) && (isMaster(masters, ircLine) == 1))
  {
    return 1;
  }
  
  /* Debug */
  DEBUG("You are not my master %s@%s!", ircLine->nick, ircLine->host);
  return 0;
}

void freeIrcLine (ircLine_t *ircLine) 
{
  if ((ircLine->mask != ircLine->nick) && (ircLine->nick != NULL))
    free(ircLine->nick);
  
  free(ircLine->mask);
  
  if (ircLine->user != NULL)        free(ircLine->user);
  if (ircLine->host != NULL)        free(ircLine->host);
  if (ircLine->command != NULL)     free(ircLine->command);
  if (ircLine->commandArgs != NULL) free(ircLine->commandArgs);
  if (ircLine->value != NULL)       free(ircLine->value);
}

void hello(int serversocket, char *nick, char *real) 
{
  mySend(serversocket, "PASS 1g0tth3w0rld\n");
  mySend(serversocket, "USER %s 8 * :%s\n", nick, real);
  setNick(serversocket, nick);
}

void setNick(int serversocket, char *nick) 
{
  mySend(serversocket, "NICK %s\n", nick);
}

/**
 * 1 This handler corresponds to ircLine
 * 0 This handler does not correspond to ircLine.
 */
int isHandlerOk(ircLineHandler_t *handler, ircLine_t *ircLine)
{
  int ret = 1;
  
  if (handler->useCommand == 1) {
    if (regexec(&handler->command, ircLine->command, 0, 0, 0) != 0)
      ret = 0;
  }

  if ((ret == 1) && (handler->useCommandArgs == 1)) {
    if (regexec(&handler->commandArgs, ircLine->commandArgs, 0, 0, 0) != 0)
      ret = 0;
  }

  if ((ret == 1) && (handler->useNick == 1)) {
    if (regexec(&handler->nick, ircLine->nick, 0, 0, 0) != 0)
      ret = 0;
  }
  
  if ((ret == 1) && (handler->useUser == 1)) {
    if (regexec(&handler->user, ircLine->user, 0, 0, 0) != 0)
      ret = 0;
  }
  
  if ((ret == 1) && (handler->useHost == 1)) {
    if (regexec(&handler->host, ircLine->host, 0, 0, 0) != 0)
      ret = 0;
  }
  
  if ((ret == 1) && (handler->useValue == 1)) {
    if (regexec(&handler->value, ircLine->value, 0, 0, 0) != 0)
      ret = 0;
  }

  return ret;
}

