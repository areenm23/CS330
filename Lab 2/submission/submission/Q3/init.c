#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main (int argc, char **argv) {

    /**
     * TODO: Write your code here.
     */



if(argc != 3){
       	perror("Error"); return 1;
}
int fd = open(argv[2], O_RDONLY);
if(fd < 0) {
 printf("Error");      	perror("Error");
return 1;
}
char* search = argv[1]; int len = strlen(search);

// printf("%d\n", len);
char* buff = malloc(1025);

ssize_t r; int found = 0;
char line[2048];

int pos = 0;

while((r = read(fd, buff, sizeof buff)) > 0) {
	for(int i=0; i<r; i++) {
          line[pos++] = buff[i];
if(buff[i] == '\n' || pos > 2046) {
line[pos] = '\0';
if(strstr(line, search)) {

found = 1; break;

}
pos = 0;
}

	}
if(found) break;
}


free(buff);
close(fd);
if(found) printf("FOUND\n");
else printf("NOT FOUND\n");


    return 0;
}
