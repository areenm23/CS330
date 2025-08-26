#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include <fcntl.h>
#include <string.h>
int main(int argc, char **argv)
{
    /**
     * TODO: Write your code here.
     */

if(argc != 2) {

perror("error"); return 1;
}

int fd = open(argv[1], O_RDONLY);
if(fd < 0) {
perror("error"); return 1;
}

//char buff[1];
ssize_t r;
int rd = 0, clse = 0, op = 0,  stat = 0, write = 0, execve = 0;
int MAX = 2048;
 char buf[1025]; int pos = 0;
char line[2048];

while((r = read(fd, buf, sizeof buf)) > 0) {
//  printf("%s\n", buf+7);
for(int i=0; i<r; i++) {
char c = buf[i];

if(c != '\n')  {
if(pos < MAX-1) line[pos++] = c; 
continue;
}


line[pos] = '\0';

if(strstr( line, " openat(") ) op++;
else if(strstr( line, " close(")) clse++;
else if(strstr(line, " stat(")) stat++;
else if(strstr(line, " write(")) write++;
else if(strstr(line, " execve(")) execve++;
else if(strstr(line, " read(")) rd++;

pos = 0;
}



}


if(r < 0) exit(1);
if(pos > 0) {

line[pos] = '\0';

if(strncmp("openat", line, 6) == 0) op++;
else if(strncmp("close", line, 5) == 0) clse++;
else if(strncmp("stat", line, 4) == 0) stat++;
else if(strncmp("write", line , 5) == 0) write++;
else if(strncmp("execve", line, 6) == 0) execve++;
else if(strncmp("read", line, 4) == 0) rd++;



}



printf("openat: %d\n", op);
printf("close: %d\n", clse);
printf("read: %d\n", rd);
printf("write: %d\n", write);
printf("stat: %d\n", stat);
printf("execve: %d\n", execve);

}
