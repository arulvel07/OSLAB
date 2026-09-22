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
        // --- CHILD PROCESS ---
        close(p2c[1]); // Close unused write end of parent->child
        close(c2p[0]); // Close unused read end of child->parent

        int num1, num2;
        char op;
        read(p2c[0], &num1, sizeof(int));
        read(p2c[0], &num2, sizeof(int));
        read(p2c[0], &op, sizeof(char));
        close(p2c[0]);

        double result = 0.0;
        int status_code = 0; // 0: OK, 1: Division by zero, 2: Invalid operator

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
                    status_code = 1;
                } else {
                    result = (double)num1 / num2;
                }
                break;
            default:
                status_code = 2;
                break;
        }

        // Send back error status and result
        write(c2p[1], &status_code, sizeof(int));
        write(c2p[1], &result, sizeof(double));
        close(c2p[1]);
        exit(0);
    } else {
        // --- PARENT PROCESS ---
        close(p2c[0]); // Close unused read end of parent->child
        close(c2p[1]); // Close unused write end of child->parent

        int num1 = 12;
        int num2 = 4;
        char op = '+';

        // Send inputs to child
        write(p2c[1], &num1, sizeof(int));
        write(p2c[1], &num2, sizeof(int));
        write(p2c[1], &op, sizeof(char));
        close(p2c[1]); // Close write end to indicate finish

        // Receive result from child
        int status_code;
        double result;
        read(c2p[0], &status_code, sizeof(int));
        read(c2p[0], &result, sizeof(double));
        close(c2p[0]);

        // Wait to avoid leaving a zombie child
        wait(NULL);

        if (status_code == 1) {
            printf("Error: Division by zero\n");
        } else if (status_code == 2) {
            printf("Error: Invalid operator\n");
        } else {
            printf("Result of %d %c %d = %.2f\n", num1, op, num2, result);
        }
    }

    return 0;
}
