# Week 3 - Question 2: Two-Way Bidirectional Pipe IPC (Calculator Worker)

## Problem Statement
Write a C program that simulates a simple calculator worker process using two pipes for two-way communication, where the parent process sends two integers and an operator (`+`, `-`, `*`, or `/`) to the child (for example, `"12 4 +"`), the child reads these values, performs the corresponding arithmetic operation, and sends the result back to the parent, which then prints the final output in the format `"Result of 12 + 4 = 16"`; the program should use one pipe for parent-to-child communication and a separate pipe for child-to-parent communication, handle division by zero gracefully by having the child send back an error code instead of crashing, ensure all unused pipe ends are properly closed in both processes, and use `wait()` in the parent to avoid leaving a zombie child process.

![Question](./Screenshot%202026-09-22%20140148.png)

---

## Architecture: Two-Way Communication Protocol
```
Parent Process                         Child Process
  [p2c Write: p2c[1]] ===== Pipe 1 =====> [p2c Read: p2c[0]]
  [c2p Read:  c2p[0]] <==== Pipe 2 ===== [c2p Write: c2p[1]]
```
* **Unused Ends Closed in Parent:** `p2c[0]` (read) and `c2p[1]` (write).
* **Unused Ends Closed in Child:** `p2c[1]` (write) and `c2p[0]` (read).

---

## Solution Code (`Program1.c`)

```c
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
```

---

## How to Compile & Run
```bash
gcc Program1.c -o Program1
./Program1
```
