
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<string.h>
#include <sys/syscall.h>

extern int isPrime(long x); // Returns zero if composite.
                            // Returns one if prime. 

long find_primes(int num_cp, long M)
{
   	
long ans = 0;

if(num_cp == 0) return -1;
if(M < num_cp) num_cp = M;

int pipes[num_cp][2];
pid_t *childs = (pid_t*)malloc(sizeof(pid_t) * (size_t)num_cp);
int range = (M + num_cp - 1) / num_cp;
long  base = M / num_cp;
long rem = M % num_cp;
int last = 0;
for(int i=0; i<num_cp; i++) {
       int L = i*range;
       int R = (i+1)*range;
       if(R > M) R = M;

      if(syscall(SYS_pipe, pipes[i]) == -1) {
   printf("pipe is exiting\n");     free(childs); exit(1);

	}

      childs[i] = fork();

      if(childs[i] == 0) {


      close(pipes[i][0]);
      int cnt = 0;
       for(int j=L; j<R; j++) {
	       if(j == 0 || j==1) continue;
          if(isPrime(j)) cnt++;
      } 

      char buff[1024];

      sprintf(buff, "%d", cnt);

      write(pipes[i][1], &cnt, sizeof(cnt));

      close(pipes[i][1]);

exit(0);       
 }
else close(pipes[i][1]);	
} 




for(int i=0; i<num_cp; i++) {
	if(childs[i] < 0) continue;
	int status;
       long  count = 0;
//       waitpid(childs[i], &status, 0);
       read(pipes[i][0], &count, sizeof(count));
   //    printf("count%d : %ld\n", i+1, count);
       ans += count;
       close(pipes[i][0]); wait(NULL);
}

free(childs);
return ans; 

}






