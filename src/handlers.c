#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "handlers.h"
#include "my_string.h"


handlerList *createHandlerListObj() {
  handlerList *work;

  work = malloc(sizeof(handlerList));
  if (work == NULL) {
    fprintf(stderr, "Error in malloc: Out of memory.\n");
    exit(1);
  }
  work->state    = 0; /* 0 Inactive */
  work->handler  = NULL;
  work->function = NULL;
  work->module   = NULL;
  work->next     = NULL;
  return work;
}

/**
 * remove an handler from a list
 *
 */
handlerList *removeHandlerListElement(handlerList *list, handlerList *element)
{
  handlerList *hPrev = NULL, *ret = list;

  while (list != NULL)
  {
    if ( list->handler == element->handler )
    {
      if ( hPrev != NULL ) 
        hPrev->next = list->next;
      else
        ret = list->next;

      break;
    }
    hPrev = list;
    list = list->next;
  }
  
  return ret;
}


/* Used to add a handler to a module */
handlerList *addModHandler(handlerList *start, handlerList *handler)
{
  handlerList *work;
  handlerList *newElem;
  
  /* Create the new element */
  newElem = malloc(sizeof(handlerList));
  newElem->next     = NULL;
  newElem->module   = handler->module;
  newElem->function = handler->function;
  newElem->state    = handler->state;
  newElem->handler  = handler->handler;
  
  work = start;
  /* Positioning on the last element */
  if (work != NULL)
  {
    while (work->next != NULL) 
      work = work->next;

    work->next = newElem;
  }
  else
    return newElem;

  return start;
}

/* Returns 0 if it's possible to add the handler. */
/* TODO: MODIFY THIS WRONG FUNCTION */
int canAddHandler(handlerList *globalHandList, handlerList *handler) {
  int retCode = 0;
  
  while (globalHandList != NULL) 
  {
    if (globalHandList == handler)
    {
      fprintf(stderr, "a module already exists with that handler.\n");
      retCode = 1;
      break;
    }
    
    globalHandList = globalHandList->next;
  }
  
  return retCode;
}
