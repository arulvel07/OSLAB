#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Child executes Linux wc command to compute line, word, and character counts
        printf("[Child PID %d] Running 'wc' on file: %s\n", getpid(), argv[1]);
        execlp("wc", "wc", argv[1], (char *)NULL);

        // execlp only returns if an error occurs
        perror("execlp failed");
        exit(1);
    }

    // Parent continues and waits for child
    printf("Parent (PID %d): Waiting for child to finish analysis...\n", getpid());
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("Parent: Child finished with exit status %d.\n", WEXITSTATUS(status));
    }

    return 0;
}
