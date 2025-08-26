#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/time.h>
// #include<sys/wait.h>


extern int isPrime(long x); // Returns zero if composite.
                            // Returns one if prime. You may use this if you want. 
int find_primes(long M)
{
  //Your code goes in here

pid_t rc1 = fork();
 

if(rc1 < 0) {

      perror("error in creating process");
}

// child1

if(rc1 == 0) {
        int ans1 = 0;
	for(long x=2; x<=M/2; x++) {
            if(isPrime(x)) ans1++;
	}
	ans1%=256;
	exit(ans1);

}
 pid_t rc2 = fork();

if(rc2 < 0) {

      perror("error in creating process");
}


// child2

if(rc2 == 0) {

  int ans2 = 0;
  for(long x=M/2+1; x<=M; x++) {
       if(isPrime(x)) ans2++;
   } 
   ans2 %= 256;
   exit(ans2);
}

// parent

int status1=0, status2 = 0;
if (rc1 > 0 && rc2 > 0) {
	
    pid_t wc1 = waitpid(rc1, &status1, 0);
    pid_t wc2 = waitpid(rc2, &status2, 0);

//  printf("total: %d\nstatus1: %d status2: %d\n", status1+status2, status1, status2);

}


return (WEXITSTATUS(status1)+WEXITSTATUS(status2))%256;

}



