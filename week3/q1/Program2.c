//CS24i1027
//Arulvel
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(){
    int pipefd[2];
    char buffer[100];
    
    pipe(pipefd);
    pid_t pid = fork();
    
    if(pid>0){ //parent- writes
        close(pipefd[0]);
        write(pipefd[1], "Hello, Child!", 14);
        close(pipefd[1]);
        wait(NULL);
    }
    
    else if(pid==0){ //child- reads
        close(pipefd[1]);
        int n=read(pipefd[0], buffer, sizeof(buffer)-1);
        if(n>0){
            buffer[n] = '\0';
            printf("Child received: %s\n", buffer);
        }
           
        close(pipefd[0]);
    }
    return 0;
}