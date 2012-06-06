#ifndef _MODULES_H
#define _MODULES_H

#include "handlers.h"

typedef struct myModule {
  /* This identify the host version */
  char *module_version;
  /* handlers */
  handlerList *handlers;
  /* A short name of the module */
  char *moduleName;
  /* A short description of what the modules do */
  char *moduleDescription;
  /* Module descriptor */
  void *md;
} myModule;

typedef struct moduleList {
  /* Module Descriptor */
  void *md;
  /* State: 1 = loaded & functionnal, 0 loaded or not, but not functional. */
  int state;
  struct moduleList *next;
  myModule *mod;
} moduleList;

myModule *getModuleByName (const moduleList *start, const char *modName);

void freeModuleListElement(moduleList *list);
moduleList *removeModuleListElement(moduleList *list, myModule *mod);

int isModuleExistant(const char *modPath, const char *modName, char **finalPath);

myModule *registerModule (handlerList **startHandlerList, moduleList **start, const char *modPath, const char *modName);

moduleList *unloadModule (handlerList **startHandlerList, moduleList **start, myModule *mod);
void *loadModule(const char *modPath);

int testModule (handlerList **startHandlerList, moduleList *start, const myModule *mod, const char *host_version);

void *execFunction(void *md, const char *symbol, void *args);


#endif
