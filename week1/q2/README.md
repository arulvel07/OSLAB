# Week 1 - Question 2: Create 8 Processes Using 3 `fork()` Calls

## Problem Statement
Using exactly three `fork()` system calls, write a C program to create 8 processes also display the PID of each process.

![Question](./Screenshot%202026-09-22%20135559.png)

---

## Explanation
Each call to `fork()` doubles the number of active processes:
- Initial process: $2^0 = 1$
- After 1st `fork()`: $2^1 = 2$ processes
- After 2nd `fork()`: $2^2 = 4$ processes
- After 3rd `fork()`: $2^3 = 8$ processes

---

## Solution Code (`q2.c`)

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main() {
    pid_t x = fork();
    pid_t y = fork();
    pid_t z = fork();

    printf("PID:- %d\n", getpid());
    return 0;
}
```

---

## How to Compile & Run
```bash
gcc q2.c -o q2
./q2
```
