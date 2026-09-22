# Week 1 - Question 5: Create Exactly 4 Processes Using 3 `fork()` Calls

## Problem Statement
Use 3 `fork()` system calls to create 4 processes exactly and print the process tree that contains the PID of parent process along with the 3 child processes.

![Question](./Screenshot%202026-09-22%20135731.png)

---

## Explanation
To generate exactly 4 processes (1 parent + 3 children) instead of 8:
- The `fork()` calls are nested inside `if (fork() != 0)`.
- Because `fork()` returns a non-zero PID to the parent and `0` to the child, only the parent continues into the nested `if` statements to spawn the next child.
- Children immediately drop out to the print statement without calling further forks.

---

## Solution Code (`q5.c`)

```c
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main() {
    if (fork() != 0) {
        if (fork() != 0) {
            fork();
        }
    }

    printf("PID: %d, PPID: %d\n", getpid(), getppid());
    return 0;
}
```

---

## How to Compile & Run
```bash
gcc q5.c -o q5
./q5
```
