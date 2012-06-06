#ifndef NET_H
#define NET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

#include "debug.h"
#include "net.h"
#include "mybot.h"

int mySend(int socket, char *format, ...)
{
  char *z_format, *buffer;
  va_list ap;
  int len, try;
  int retval;
  
  len = strlen(format) + 512; /* Try */
  z_format = (char *) malloc(len);
  if (z_format == NULL) 
  {
    perror("malloc");
    exit(1);
  }

  while ( 1 ) 
  {
    va_start(ap, format);
    try = vsnprintf (z_format, len, format, ap);
    va_end(ap);
    if (try > -1 && try < len)
      break;
    if (try > -1)
      len = try+1;
    else
      len *= 2; /* Uggly =) */
    z_format = (char *) realloc((void *) z_format, len);
    if (z_format == NULL)
    {
      (void) perror("realloc");
      (void) exit(1);
    }
  }

  /* Debug */
  len = strlen(z_format);
  buffer = (char *) malloc(len);
  (void) strncpy(buffer, z_format, len-1); /* strips the last \n */
  buffer[len-1] = '\0';
  DEBUG("Sending `%s'", buffer);
  (void) free(buffer);

  len = strlen(z_format);
  retval = send(socket, z_format, len, 0);
  if (retval < len)
    D_ERROR("Warning: message not fully send. (%d of %d bytes)", retval, len);

  (void) free(z_format);
  return retval;
}

int server_connect (const char *servername, const char *serverport)
{
  struct addrinfo hints, *res, *ressave;
  int server_socket = -1;
  int n;
  
  DEBUG("Entring Server_connect");
  DEBUG("Server: %s:%s", servername, serverport);
  
  /* on initialise la socket */
  (void) memset(&hints, 0, sizeof(struct addrinfo));
  
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  n = getaddrinfo(servername, serverport, &hints, &res);
  if (n < 0) {
    D_ERROR("getaddrinfo error: [%s]", gai_strerror(n));
    return -1;
  }

  ressave = res;
  while (res) {
    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (!(server_socket < 0)) {
      if (connect(server_socket, res->ai_addr, res->ai_addrlen) == 0)
        break;
      (void) close(server_socket);
      server_socket = -1;
    }
    res = res->ai_next;
  }
  
  (void) freeaddrinfo(ressave);
  
  if (server_socket < 0)
  {
    D_ERROR("Error during connection: server_socket < 0.");
    return -1;
  }
  
  FD_ZERO(&rfds);
  FD_SET(server_socket, &rfds);

  return server_socket;
}

int server_close (int serversocket)
{
  DEBUG("Closing server connection.");
  (void) shutdown(serversocket, 2);
  (void) close(serversocket);
  serversocket = -1;
  DEBUG("Server connection closed.");
  return -1;
}

int server_listen(const char *hostname,
                  const char *service,
                  int        familly,
                  int        socktype) 
{
  struct addrinfo hints, *res, *ressave;
  int err;
  int sockfd = -1;

  (void) memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_flags    = AI_PASSIVE;
  hints.ai_family   = familly;
  hints.ai_socktype = socktype;

  if ( (err = getaddrinfo(hostname, service, &hints, &res)) < 0)
  {
    D_ERROR("Error in getaddrinfo: %s", gai_strerror(err));
    return -1;
  }

  /* backuping result from getaddrinfo */
  ressave = res;

  while (res)
  {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (!(sockfd < 0))
    {
      if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0)
        break;
      
      /* On ne peut pas binder */
      (void) close(sockfd);
      sockfd = -1;
    }
    res = res->ai_next;
  }

  if (sockfd < 0)
  {
    (void) freeaddrinfo(ressave);
    D_ERROR("Unable to bind socket.");
    return -1;
  }

  (void) listen(sockfd, 256);
  
  (void) freeaddrinfo(ressave);

  return sockfd;
}

#endif
