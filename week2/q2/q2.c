#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <backup_destination>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process: execute Linux cp command 
        execlp("cp", "cp", argv[1], argv[2], (char *)NULL);

        // execlp() returns only if an error occurs
        perror("execlp");
        exit(1);
    }

    // Parent process continues doing other work
    printf("Backup started. Child PID: %d\n", pid);
    printf("Parent process can continue logging/other work...\n");

    // Wait for the backup to finish
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        printf("Backup completed successfully.\n");
    else
        printf("Backup failed.\n");

    return 0;
}


