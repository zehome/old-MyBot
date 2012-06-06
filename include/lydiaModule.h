#ifndef LYDIAMODULE_H
#define LYDIAMODULE_H

#include "common.h"

#define MODNAME "lydiaModule"
#define MODULE_VERSION "0.1"
#define MODDESC "Ce module est juste la pour faire un test..."

myModule *init();
handlerList *createHandler(myModule *mod);

char *lydia(void *args);

#endif
