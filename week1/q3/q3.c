#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

// Approach 1 (Recommended / Best):
// Deterministic linear process chain: exactly 5 processes created (strictly < 16)
int main() {
    pid_t pid;

    printf("Original process PID: %d\n", getpid());

    pid = fork();  // fork 1
    if (pid == 0) {
        printf("Process 2 PID: %d (parent PID: %d)\n", getpid(), getppid());

        pid = fork();  // fork 2
        if (pid == 0) {
            printf("Process 3 PID: %d (parent PID: %d)\n", getpid(), getppid());

            pid = fork();  // fork 3
            if (pid == 0) {
                printf("Process 4 PID: %d (parent PID: %d)\n", getpid(), getppid());

                pid = fork();  // fork 4
                if (pid == 0) {
                    printf("Process 5 PID: %d (parent PID: %d)\n", getpid(), getppid());
                }
            }
        }
    }

    return 0;
}

/*
// Approach 2: 4 Unconditional forks with PID display filter (prints 15 child PIDs)
int main_alt() {
    pid_t original = getpid();

    fork(); // fork 1
    fork(); // fork 2
    fork(); // fork 3
    fork(); // fork 4

    if (getpid() != original) {
        printf("Process PID: %d\n", getpid());
    }

    return 0;
}
*/