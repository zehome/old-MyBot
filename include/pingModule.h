#ifndef PINGMODULE_H
#define PINGMODULE_H

#include "common.h"

/* This is module name ... */
#define MODNAME "pingModule"
#define MODULE_VERSION "0.1"
#define MODDESC "Ce module est juste la pour faire un test..."

myModule *init();
handlerList *createHandler(myModule *mod);

char *ping_pong(void *args);

#endif
