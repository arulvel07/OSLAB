#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(){
    if(fork()!=0){
        if(fork()!=0){
            fork();
        }
    }

    printf("PID: %d, PPID: %d\n", getpid(), getppid());
}
