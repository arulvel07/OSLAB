# Week 1 - Question 4: Zombie Process Lifecycle & Eradication

## Problem Statement
Create a child process that prints its PID and exits immediately. In the parent, sleep for 10 seconds without calling `wait()`, then run `ps` to show the child is now a zombie (Z state). After the sleep, call `wait()` and show the zombie disappears.

![Question](./Screenshot%202026-09-22%20135703.png)

---

## Explanation
- **Zombie Process:** When a child calls `exit()`, its resources are freed, but its entry in the process table (PCB) remains so the parent can read its exit status.
- While the parent is asleep without having called `wait()`, inspecting `ps -o pid,ppid,state,cmd` shows the child with state `'Z'` (or `defunct`).
- Once `wait(NULL)` is executed by the parent, the kernel removes the child process entry completely.

---

## Solution Code (`q4.c`)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        printf("Child PID:- %d\n", getpid());
        exit(0); // child exits immediately
    } else { // parent
        printf("parent pid:- %d\n", getpid());
        printf("child pid:- %d\n", pid);
        sleep(10); // parent sleeps without calling wait yet
        
        printf("\nafter 10 seconds\n");
        wait(NULL); // reap zombie child
        
        printf("\nzombie removed after wait().\n");
    }
    return 0;
}
```

---

## How to Verify
1. Compile and execute:
   ```bash
   gcc q4.c -o q4
   ./q4
   ```
2. While the parent is in the 10-second sleep, open a second terminal and run:
   ```bash
   ps -o pid,ppid,state,cmd
   ```
   Notice state `Z+` next to the child PID.
3. Once the 10 seconds elapse and `wait()` finishes, run `ps` again to observe the child has been reaped.
