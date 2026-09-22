#include <stdio.h>
#include <unistd.h>

int main(){
    
    fork();
    fork();
    fork();
    
    if(getpid()%2==0) fork();
    printf("process id (pid):- %d\n", getpid());
    return 0;
    
}