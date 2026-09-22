# Week 3 - Question 1: Unidirectional Anonymous Pipe IPC

## Problem Statement
Write a C program that creates a pipe and then uses `fork()` to create a child process; the parent process should write the string `"Hello, Child!"` into the pipe, while the child process reads that message from the pipe and prints it to the screen in the format `"Child received: Hello, Child!"` — remembering that since only one-way communication is needed (parent to child), each process should close the pipe end it doesn't use.

![Question](./Screenshot%202026-09-22%20140124.png)

---

## Explanation
- An anonymous pipe provides a half-duplex (unidirectional) data channel between related processes.
- `pipefd[0]` is opened for reading; `pipefd[1]` is opened for writing.
- **Rule of Least Privilege / Deadlock Prevention:**
  - The parent only writes: it closes `pipefd[0]` before writing, and closes `pipefd[1]` when finished.
  - The child only reads: it closes `pipefd[1]` before reading, and closes `pipefd[0]` when finished.

---

## Solution Code (`Program2.c`)

```c
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    char buffer[100];
    
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }
    
    if (pid > 0) { 
        // Parent: writes message
        close(pipefd[0]); // Close unused read end
        write(pipefd[1], "Hello, Child!", 14);
        close(pipefd[1]); // Close write end to signal EOF
        wait(NULL);       // Wait for child to exit
    } else { 
        // Child: reads message
        close(pipefd[1]); // Close unused write end
        int n = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("Child received: %s\n", buffer);
        }
        close(pipefd[0]);
    }
    return 0;
}
```

---

## How to Compile & Run
```bash
gcc Program2.c -o Program2
./Program2
```
