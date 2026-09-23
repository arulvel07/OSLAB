// CS24i1027
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int p2c[2]; // Parent to Child pipe
    int c2p[2]; // Child to Parent pipe

    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // ================= CHILD PROCESS =================
        close(p2c[1]); // Close unused write end of parent->child
        close(c2p[0]); // Close unused read end of child->parent

        int num1, num2;
        char op;
        read(p2c[0], &num1, sizeof(int));
        read(p2c[0], &num2, sizeof(int));
        read(p2c[0], &op, sizeof(char));
        close(p2c[0]); // Finished reading input

        double result = 0.0;

        switch (op) {
            case '+':
                result = num1 + num2;
                break;
            case '-':
                result = num1 - num2;
                break;
            case '*':
                result = num1 * num2;
                break;
            case '/':
                if (num2 == 0) {
                    close(c2p[1]);
                    exit(1); // Exit code 1: Division by zero error
                }
                result = (double)num1 / num2;
                break;
            default:
                close(c2p[1]);
                exit(2); // Exit code 2: Invalid operator error
        }

        // On success: send ONLY the calculated result back through the pipe
        write(c2p[1], &result, sizeof(double));
        close(c2p[1]);

        exit(0); // Exit code 0: Normal successful execution
    } else {
        // ================= PARENT PROCESS =================
        close(p2c[0]); // Close unused read end of parent->child
        close(c2p[1]); // Close unused write end of child->parent

        int num1 = 12;
        int num2 = 4;
        char op = '+';

        // Send inputs to child via pipe 1
        write(p2c[1], &num1, sizeof(int));
        write(p2c[1], &num2, sizeof(int));
        write(p2c[1], &op, sizeof(char));
        close(p2c[1]); // Close write end to signal input transmission is complete

        // Wait for child to exit and capture its exit status code
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);

            if (exit_code == 1) {
                printf("Error: Division by zero! (Child exited with status %d)\n", exit_code);
            } else if (exit_code == 2) {
                printf("Error: Invalid operator! (Child exited with status %d)\n", exit_code);
            } else if (exit_code == 0) {
                // Success: read computed result from pipe 2
                double result;
                read(c2p[0], &result, sizeof(double));
                printf("Result of %d %c %d = %.2f\n", num1, op, num2, result);
            } else {
                printf("Child failed with unexpected exit status: %d\n", exit_code);
            }
        } else {
            printf("Child process terminated abnormally.\n");
        }

        close(c2p[0]); // Clean up read descriptor
    }

    return 0;
}
