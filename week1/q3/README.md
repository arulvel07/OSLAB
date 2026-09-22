# Week 1 - Question 3: Create Less Than 16 Processes Using 4 `fork()` Calls

## Problem Statement
Write a C program to create less than 16 processes using 4 `fork()` calls also display the PID of each process.

![Question](./Screenshot%202026-09-22%20135631.png)

---

## Explanation
If 4 consecutive unconditional `fork()` calls were executed, it would generate $2^4 = 16$ processes. By making the 4th `fork()` conditional (e.g., executing only if `getpid() % 2 == 0` or restricted to a specific child), strictly less than 16 processes are created.

---

## Solution Code (`q3.c`)

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    fork();
    fork();
    fork();
    
    // Only processes with an even PID execute the 4th fork
    if (getpid() % 2 == 0) {
        fork();
    }
    
    printf("process id (pid):- %d\n", getpid());
    return 0;
}
```

---

## How to Compile & Run
```bash
gcc q3.c -o q3
./q3
```
