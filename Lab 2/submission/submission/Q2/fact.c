#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/time.h>
#include<sys/wait.h>

int main(int argc, char **argv)
{
   /*Your code goes here */


if(argc < 2) 
perror("invalid number of arguments");



long n = atol(argv[1]);
long acc = (argc == 2) ? 1 : atol(argv[2]);


if(n < 2) {
printf("%ld", acc); return 1;
}



char nextN[20], nextacc[20];
sprintf(nextN, "%ld", n-1);
sprintf(nextacc, "%ld", acc*n);

execl("./fact", "fact", nextN, nextacc, NULL);
perror("execl failed");

   return 1;
}
