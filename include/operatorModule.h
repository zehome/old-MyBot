#ifndef LISTMODULE_H
#define LISTMODULE_H

#include "common.h"

/* This is module name ... */
#define MODNAME "operatorModule"
#define MODULE_VERSION "0.1"
#define MODDESC "This module can do some standard IRC operations, such as changing modes."

myModule *init();
handlerList *createHandler(myModule *mod);

#endif
