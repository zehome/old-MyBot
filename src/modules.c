#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

#include "debug.h"
#include "modules.h"
#include "my_string.h"

#define MYBOT_VERSION "0.1"

myModule *getModuleByName(const moduleList *start, const char *modName) 
{  
  while (start != NULL) 
  {
    if (mystr_eq(start->mod->moduleName, modName) == 1) 
      return start->mod;

    start = start->next;
  }
  
  DEBUG("Unable to find module %s", modName);

  return NULL;
}

/* Returns:
 * 1 module existant
 * 0 inexistant.
 */
int isModuleExistant(const char *modPath, const char *modName, char **finalPath)
{
  FILE *file;
  int retour = 1;
  
  *finalPath = myprintf("%s/%s.so", modPath, modName);
  
  if ((file = fopen(*finalPath, "rb")) == NULL)
    retour = 0;
  else
  {
    fclose(file);
    retour = 1;
  }
  
  return retour;
}

myModule *registerModule(handlerList **startHandlerList, moduleList **start, const char *modPath, const char *modName) 
{
  moduleList *work;
  moduleList *newListElem;
  myModule *mod;
  myModule *(*initFunction)();
  void *md;
  char *finalModPath;

  if ( isModuleExistant(modPath, modName, &finalModPath) != 1)
  {
    fprintf(stderr, "Failed to open %s/%s.so\n", modPath, modName);
    free(finalModPath);
    return NULL;
  }
  
  if ( (md = loadModule(finalModPath)) == NULL) 
  {
    fprintf(stderr, "An error occured during module loading.\n");
    free(finalModPath);
    return NULL;
  }
  free(finalModPath);

  initFunction = dlsym(md, "init");
  if (initFunction == NULL) {
    /* a module should NEVER have a NULL init function */
    fprintf(stderr, "Error loading module: symbol init == NULL or not fould.\n");
    dlclose(md);
    return NULL;
  }
  
  mod = (*initFunction)();
  mod->md = md;
  DEBUG("Now testing the module ...");
  if (testModule(startHandlerList, *start, mod, MYBOT_VERSION) != 0) {
    fprintf(stderr, "moduleVerification Failed.\n");
    fprintf(stderr, "Module not loaded.\n");
    unloadModule(startHandlerList, start, mod);
    return NULL;
  }

  /* Creating the new Module List Element */
  newListElem = malloc(sizeof(moduleList));
  /* TODO: check */
  newListElem->md     = mod->md;
  newListElem->state  = 1; /* Loaded & Operational */
  newListElem->next   = NULL;
  newListElem->mod    = mod;

  /* Getting the last entry in the list */
  work = *start;
  if (work == NULL) {
    *start = newListElem;
    work = newListElem;
  } else {
    while (work->next != NULL) work = work->next;
    work->next = newListElem;
  }
  
  DEBUG("module %s successfully registered.", newListElem->mod->moduleName);

  return work->mod;
}

/* Wrapper for dlopen functions */
void *loadModule(const char *modPath) 
{
  void *md;

  md = dlopen(modPath, RTLD_LAZY);
  if (md == NULL) {
    fprintf(stderr, "Error loading module %s\n", modPath);
    fprintf(stderr, "%s\n", dlerror());
  } else {
    DEBUG("Module %s successfully loaded.", modPath);
  }

  return md;
}

void freeModuleListElement(moduleList *list)
{
  myModule *mod = list->mod;
  
  /* close module */
  if (dlclose(mod->md) != 0)
    fprintf(stderr, "Module unregistered, but can't be unloaded. dlclose error.\n");
  else
    DEBUG("Module successfully unregistered & unloaded.");

  /* free module */
  if (mod->moduleName)        free(mod->moduleName);
  if (mod->moduleDescription) free(mod->moduleDescription);
  free(mod);

  /* free list */
  free(list);
}  

/**
 * remove a module from a list
 *
 */
moduleList *removeModuleListElement(moduleList *list, myModule *mod)
{
  moduleList *prev = NULL, *ret = list;

  while ( list != NULL )
  {
    if ( list->mod == mod )
    {
      if ( prev != NULL )
        prev->next = list->next;
      else
        ret        = list->next;

      /* free mod */      
      freeModuleListElement(list);      

      break;
    }

    prev = list;
    list = list->next;
  }

  return ret;
}

moduleList *unloadModule(handlerList **startHandlerList, moduleList **start, myModule *mod) 
{
  /* Handler List of a module */
  handlerList *modHdrList, *modHdrListNext;
  
  /* Deleting module's handlers from global handler List */
  modHdrList = mod->handlers;
  while (modHdrList != NULL)
  {
    modHdrListNext = modHdrList->next;

    /* remove this handler on global handler list */
    *startHandlerList = removeHandlerListElement(*startHandlerList, modHdrList);

    modHdrList = modHdrListNext;
  }
 
  /* Deleting module from global module list */
  *start = removeModuleListElement(*start, mod);

  return *start;
}

int testModule(handlerList **startHandlerList, moduleList *start, const myModule *mod, const char *host_version) 
{
  moduleList *work;
  handlerList *hdr;
  int handlerCount;
  
  /* Vérification de la version du module / hote */
  if (mystr_eq(host_version, mod->module_version) != 1) {
    fprintf(stderr, "Module version != edweb version.\n");
    return -1;
  }
  DEBUG("Module version seems good.");
  /* Fin vérif */

  /* Vérification du nom de module */
  work = start;
  while (work != NULL) {
    if (mystr_eq(work->mod->moduleName, mod->moduleName) == 1) {
      /* Module already loaded */
      DEBUG("Module is already loaded.");
      return -2;
    }
    work = work->next;
  }

  /* THe module is not loaded ... */
  DEBUG("Module seems not loaded ...");
  
  /* Handlers verification */
  
  handlerCount = 0;

  if (*startHandlerList == NULL) {
    DEBUG("No valid handlerList found. Considering all handlers OK.");
  }

  hdr = mod->handlers;
  while (hdr != NULL) {
    DEBUG("Registering handler.");
    *startHandlerList = addModHandler(*startHandlerList, hdr);
    handlerCount++;

    hdr = hdr->next;
  }
  
  DEBUG("Handlers registered: %d", handlerCount);
  
  if (handlerCount == 0) {
    DEBUG("Not any handler registered. Can't register the module.");
    return -1;
  }
  
  DEBUG("Module verification: OK.");
  return 0;
}

void *execFunction(void *md, const char *symbol, void *args) 
{
  void *(*function)(void *args);

  function = dlsym(md, symbol);
  if (function == NULL) 
  {
    DEBUG("Unable to find symbol %s.", symbol);
    return NULL;
  }
  
  return (*function)(args);
}
