#include "my_string.h"

int mystr_eq(const char *str1, const char *str2)
{
  int len1, ret = 1;
  if ((str1 == NULL) || (str2 == NULL))
    ret = 0;
 
  if (ret == 1)
  {
  /* 
    int i;
    printf("str1: `%s' str2: `%s'\n", str1, str2);
    for(i = 0; i < strlen(str1)+1; i++) printf("%x ", str1[i]);
    printf("\n");
    for(i = 0; i < strlen(str2)+1; i++) printf("%x ", str2[i]);
    printf("\n");
  */
    len1 = strlen(str1);
    if((len1 != strlen(str2)) || (strncmp(str1, str2, len1) != 0))
      ret = 0;
  }

  return ret;
}

char *my_strndup(const char *src, int size)
{
  char *str;
  
  str = calloc(size+1, sizeof(char));
  if (str == NULL) {
    perror("calloc");
    exit(1);
  }

  strncpy(str, src, size);
  
  return str;
}


char *my_word_next(char *str)
{
  /* search end of word */
  while((*str != ' ') && (*str != '\0') && (*str != '\n')) str++;

  /* goto next word */
  while(*str == ' ') str++;

  return str;
}

char *strip_crlf(char *str)
{
  char *modified, *backup;
  
  backup = modified = (char *) malloc(strlen(str));
  
  while(*str != '\0')
  {
    if ((*str != '\n') && (*str != '\r'))
      *modified++ = *str;

    str++;
  }

  *modified = '\0';

  return backup;
}

int my_word_len(const char *str)
{
  int ret = 0;

  /* search end of word */
  while((str[ret] != '\0') && (str[ret] != ' ') && (str[ret] != '\n'))
    ret++;

  return ret;
}


char *my_word_dup(const char *str)
{
  int len;
  
  if (str == NULL)
    return NULL;

  len = my_word_len(str);
  return my_strndup(str, len);
}

char *my_word_num(char *str, int num)
{
  int i;
  char *next;
  char *word = NULL;
  
  if (num == 1)
    return my_word_dup(str);
  
  next = str;
  for (i = 1; i < num; i++)
  {
    if (word != NULL) free(word);
   
    next = my_word_next(next);
    if ((next == NULL) || (*next == '\0') || (*next == '\n'))
    {
      word = NULL;
      break;
    }
    word = my_word_dup(next);
  }
  
  return word;
}

char *myprintf(const char *format, ...)
{
  char *z_format;
  va_list ap;
  int len, try;

  len = strlen(format) + 512; /* Try */
  z_format = malloc(len);
  if (z_format == NULL) {
    perror("malloc");
    exit(1);
  }

  while ( 1 ) {
    va_start(ap, format);
    try = vsnprintf (z_format, len, format, ap);
    va_end(ap);
    if (try > -1 && try < len)
      break;
    if (try > -1)
      len = try+1;
    else
      len *= 2; /* Ugly =) */
    z_format = realloc(z_format, len);
    if (z_format == NULL) {
      perror("realloc");
      exit(1);
    }
  }

  return z_format;
}
