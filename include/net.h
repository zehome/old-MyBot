#ifndef NET_H
#define NET_H

int server_connect (const char *servername, const char *serverport);
int server_close (int serversocket);
int mySend(int socket, char *format, ...);
int server_listen(const char *hostname, const char *service, int familly, int socktype);

#endif
