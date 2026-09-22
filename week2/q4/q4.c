#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(){
    pid_t pid = fork();
    
    if(pid<0){
        perror("fork failed");
        return 1;
    }
    if(pid>0){
        //parent
        printf("Parent ID:- %d\n", getpid());
        printf("parent is ending...\n");
        exit(0);
    }
    else{
        //child
        pid_t parent_id = getppid();
        
        //wait until parent dies - no sleep()
        while(getppid()==parent_id){}
        printf("Dead parent ID:- %d\n", parent_id);
        printf("New parent ID:- %d\n", getppid());
    }
    return 0;
}