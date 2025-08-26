#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

void compact(void *start, void *end)
{
  /* 
   * TODO
   * Your code goes here
   */

char *r = (char*) start;
char *w = (char*) start;
char *e = (char*) end;

while(r < e) {
if(*r != '\0') {
*w++ = *r;
}
r++;
}

int d = (int)(w - e);
if(d != 0) sbrk(d);

	  return;    
}
