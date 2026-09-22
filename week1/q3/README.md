# Week 1 - Question 3: Create Less Than 16 Processes Using 4 `fork()` Calls

## Problem Statement
Write a C program to create less than 16 processes using 4 `fork()` calls also display the PID of each process.

![Question](./Screenshot%202026-09-22%20135631.png)

---

## Concept & Analysis

If 4 consecutive unconditional `fork()` calls are executed:
$$\text{Total Processes} = 2^4 = 16$$
To strictly create **less than 16 processes**, process creation must be constrained conditionally.

Here are the two primary ways to approach this problem, followed by an important viva distinction:

---

## Approach 1: Deterministic Linear Chain (Recommended / Best Solution)

### Why this is the best solution:
* Each subsequent `fork()` is nested inside `if (pid == 0)`.
* Only the newly created child process executes the next `fork()`.
* **Exact Process Count:** $1 + 4 = 5$ processes total (strictly $< 16$).
* Prints both the process PID and parent PPID to prove the linear hierarchy.

### Source Code (`q3.c`)
```c
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

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
```

---

## Approach 2: Star Topology (Parent Spawner)

### Concept:
Only the original parent executes the forks (`if (fork() != 0)`), resulting in 1 parent + 4 direct children = 5 processes total.

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    if (fork() != 0) {
        if (fork() != 0) {
            if (fork() != 0) {
                fork();
            }
        }
    }

    printf("Process PID: %d (Parent: %d)\n", getpid(), getppid());
    return 0;
}
```

---

## Approach 3: 4 Unconditional Forks with Child-Only Output Filtering

### Concept:
Executes 4 sequential `fork()` calls and uses a check `if (getpid() != original)` so that only the **15 child processes** print their PIDs (excluding the original parent).

```c
#include <stdio.h>
#include <unistd.h>

int main() {
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
```

> **Exam / Viva Note:**  
> In Approach 3, the Linux kernel still spawns $2^4 = 16$ processes in total, but only 15 processes print. In contrast, **Approach 1 physically prevents the creation of 16 processes** by constraining the `fork()` calls inside child checks. For strict evaluation, **Approach 1 is the most academically sound solution**.

---

## How to Compile & Run
```bash
gcc q3.c -o q3
./q3
```
