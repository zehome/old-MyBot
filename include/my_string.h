#ifndef MY_STRING_H
#define MY_STRING_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

int mystr_eq(const char *str1, const char *str2);
char *my_strndup(const char *src, int size);
char *myprintf(const char *format, ...);

char *my_word_next(char *str);
char *my_word_dup(const char *str);
char *my_word_num(char *str, int num);
char *strip_crlf(char *str);
int my_word_len(const char *str);

#endif
