#ifndef LISTMODULE_H
#define LISTMODULE_H

#include "common.h"

/* This is module name ... */
#define MODNAME "listModule"
#define MODULE_VERSION "0.1"
#define MODDESC "This module print some lists."

myModule *init();
handlerList *createHandler(myModule *mod);

#endif
