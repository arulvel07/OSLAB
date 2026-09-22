//Arulvel V
//CS24i1027
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void cal(char *num1, char *op, char *num2){
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed..");
        exit(1);
    }
    if (pid == 0) {
        execl("./calc_worker", "calc_worker", num1, op, num2, (char *)NULL);
        perror("execl");
        exit(1);
    }
    printf("Started calc_worker with PID %d\n", pid);
}



int main(){
    //start both calculation, the controller doesnt wait after first fork
    cal("13", "*", "5");
    cal("40", "/", "4");
    cal("46", "-", "23");
    cal("56", "+", "14");

    //wait for both process
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    printf("All calculation completed\n");

    return 0;
}

