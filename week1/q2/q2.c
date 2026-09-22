#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
int main(){
    
    // fork();
    
    // fork();
    
    // fork();
    
    
    // printf("process id (pid):- %d\n", getpid());
    // return 0;
    
    
    
    pid_t x = fork();

    
    pid_t y = fork();

    
    pid_t z = fork();

    
    
    printf("PID:- %d\n", getpid());
    
    
    
    // pid_t x =fork();
    // if(x==-1) printf("PID doesnt exist");
    
    // pid_t y =fork();
    // if(y==-1) printf("PID doesnt exist");
    
    // pid_t z =fork();
    // if(z==-1) printf("PID doesnt exist");
    
    // printf(" PID: %d\n", getpid());
}