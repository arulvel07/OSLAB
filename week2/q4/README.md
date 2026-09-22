# Week 2 - Question 4: Orphan Process Detection Without `sleep()`

## Problem Statement
Write a program where a child is created from a parent and a child waits for a parent to end. As soon as the parent ends, child prints the dead parent's ID and also new parent's ID without sleep.

![Question](./Screenshot%202026-09-22%20140047.png)

---

## Explanation
- When a parent process terminates before its child, the child becomes an **orphan process**.
- The Linux kernel re-parents orphan processes to a system supervisor process (traditionally `init` with PID 1, or `systemd` / user subreaper on modern Linux distributions).
- Rather than guessing a duration using `sleep()`, the child records its initial parent ID via `getppid()` and executes a tight busy-wait loop `while (getppid() == parent_id);`. As soon as the parent exits, `getppid()` immediately returns the new supervisor's PID.

---

## Solution Code (`q4.c`)

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }
    if (pid > 0) {
        // Parent prints its PID and terminates immediately
        printf("Parent ID:- %d\n", getpid());
        printf("parent is ending...\n");
        exit(0);
    } else {
        // Child
        pid_t parent_id = getppid();
        
        // Wait until parent terminates without using sleep()
        while (getppid() == parent_id) { }

        printf("Dead parent ID:- %d\n", parent_id);
        printf("New parent ID:- %d\n", getppid());
    }
    return 0;
}
```

---

## How to Compile & Run
```bash
gcc q4.c -o q4
./q4
```
