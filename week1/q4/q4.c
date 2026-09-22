#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){
    pid_t pid = fork();
    if(pid==0){
        printf("Child PID:- %d\n", getpid());
        exit(0); //child exits
    }
    
    else{//parent
        printf("parent pid:- %d\n", getpid());
        printf("child pid:- %d\n", pid);
        sleep(10); //parent does not call wait yet
        
        printf("\nafter 10 seconds\n");
        wait(NULL); //collect zombie
        
        printf("\nzombie removed after wait().\n");
    }
    return 0;
}

// printf("run: ps -o pid,ppid,state,cmd\n");