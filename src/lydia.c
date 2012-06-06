#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* An uggly code :-)))) */
char *lydiaise (char *val, int len, int fact) {
  int i, j, k, l, a; 
  char *to;

  
  if (fact>4)
    a=4;
  else 
    a=fact;
  
  to = malloc(300);
  for (l=0,k=1,i=0;*val!=0;i++)
  {
    for(j = 0; j < k&& l < 298; j++, l++) *to++=*val;
    k *= a;
    if (l > 298) break;
  }
  *to='\0';
  
  return to-l;
}

char *_verlan (char *str, int len) {
  char *dst, *back;
  
  back = dst = malloc(len+1);
  str += len-1;
  while (len-- >= 0) {
    *dst++ = *str--;
  }
  *dst = '\0';

  return back;
}

char *_leet (char *str, int len) {
  char *back, *dst;

  back = dst = malloc(len*2+1); /* Max */
  
  while (len-- >= 0) {
    switch (*str) {
      case 'e': *dst++ = '3'; break;
      case 'a': *dst++ = '4'; break;
      case 'l': *dst++ = '1'; break;
      case 'i': *dst++ = '|'; break;
      case 's': *dst++ = '5'; break;
      case 'o': *dst++ = '0'; break;
      case 't': *dst++ = '7'; break;
      case 'x': *dst++ = '>'; *dst++ = '<'; break;
      case 'k': *dst++ = '|'; *dst++ = '<'; break;
      case 'L': *dst++ = '|'; *dst++ = '_'; break;
      default: *dst++ = *str;
    }
    str++;
  }
  
  *dst = '\0';

  return back;
}

int _verifPalin ( char *from, int len )
{
  int i; char c;
  i = 0;

  while (len-- > 0) {
    c = from[len];
    if (((c>'a'&&c<'z')||(c>'A'&&c<'Z')) && (c != from[i]))
      return 0;
    i++;
  }
  return 1;
}
