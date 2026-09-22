# Week 2 - Question 3: Multi-Process Calculation Worker & Controller

## Problem Statement
A company has a small standalone calculation utility (`calc_worker`) that takes two numbers and an operation as command-line arguments, and prints the result. A separate controller program (`controller.c`) needs to invoke this utility as a child process whenever a calculation request comes in — instead of embedding the calculation logic directly — so the worker can be updated/replaced independently without touching the controller.

- `calc_worker`: Accepts three command-line arguments: `num1`, `operator (+, -, *, /)`, `num2`. Prints the result.
- `controller.c`: Invokes the utility and runs at least two operations in parallel concurrently.

![Question](./Screenshot%202026-09-22%20140002.png)

---

## Solution Code

### 1. `calc_worker.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage:- %s num1 operator num2\n", argv[0]);
        return 1;
    }
    double n1 = atof(argv[1]);
    double n2 = atof(argv[3]);
    char op = argv[2][0];
    double r;
    
    switch (op) {
        case '+':
            r = n1 + n2;
            break;
        case '-':
            r = n1 - n2;
            break;
        case '*':
            r = n1 * n2;
            break;
        case '/':
            if (n2 == 0) {
                fprintf(stderr, "Division by zero\n");
                return 1;
            }
            r = n1 / n2;
            break;
        default:
            printf("Invalid operator\n");
            return 1;
    }

    printf("%.2f %c %.2f = %.2f\n", n1, op, n2, r);
    return 0;
}
```

### 2. `controller.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void cal(char *num1, char *op, char *num2) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed..");
        exit(1);
    }
    if (pid == 0) {
        execl("./calc_worker", "calc_worker", num1, op, num2, (char *)NULL);
        perror("execl");
        exit(1);
    }
    printf("Started calc_worker with PID %d\n", pid);
}

int main() {
    // Start calculations concurrently: controller does not wait after each fork
    cal("13", "*", "5");
    cal("40", "/", "4");
    cal("46", "-", "23");
    cal("56", "+", "14");

    // Barrier: wait for all child processes to complete
    wait(NULL);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    printf("All calculation completed\n");

    return 0;
}
```

---

## How to Compile & Run
```bash
gcc calc_worker.c -o calc_worker
gcc controller.c -o controller
./controller
```

---

## Output Screenshot
![Output](./OUTPUT)
