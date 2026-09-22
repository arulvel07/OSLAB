# Week 4 - Question 3: Distributed Matrix Multiplier Using Worker Processes

## Problem Statement
**Concept:** Parallel computation, non-overlapping memory partition access.  
**API:** System V (`shmget`, `shmat`, `shmdt`, `shmctl`) or POSIX Shared Memory (`shm_open`, `mmap`) + `fork()` / `wait()`.

### Problem Statement:
Multiply two large $N \times N$ matrices ($A$ and $B$) using $K$ independent worker processes. The parent process allocates shared memory for Matrix $A$, Matrix $B$, and the resulting Matrix $C$. The parent forks $K$ children, assigning distinct row slices of $C$ to each worker:

$$C[i][j] = \sum_{k=0}^{N-1} A[i][k] \cdot B[k][j]$$

- Because each worker writes to a mutually exclusive row partition of Matrix $C$, computations proceed concurrently without locks or semaphores.
- The parent waits for all $K$ children to terminate, verifies the product matrix $C$, and cleans up all shared memory resources.

### Initialization & Verification:
- Initialize Matrix $A$ with all elements set to `1.0`.
- Initialize Matrix $B$ as an Identity Matrix ($B[i][j] = 1.0$ if $i == j$, else $0.0$).
- **Verification Rule:** Since $C = A \times I = A$, every element in $C$ should evaluate to `1.0`.
- If every element in $C$ equals `1.0`, print: `Verification: SUCCESS`, otherwise `Verification: FAILED`.

![Question](./Screenshot%202026-09-22%20140310.png)

---

## Solution Code (`matrix_multi_shm.c`)

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main() {
    int N, K;
    printf("Enter value of N:- ");
    if (scanf("%d", &N) != 1) return 1;
    
    printf("Enter no of workers(K<=%d):- ", N);
    if (scanf("%d", &K) != 1) return 1;
    
    int s = N * N * sizeof(double);
    int sa = shmget(IPC_PRIVATE, s, 0666 | IPC_CREAT);
    int sb = shmget(IPC_PRIVATE, s, 0666 | IPC_CREAT);
    int sc = shmget(IPC_PRIVATE, s, 0666 | IPC_CREAT);
    
    double *A = (double *)shmat(sa, 0, 0);
    double *B = (double *)shmat(sb, 0, 0);
    double *C = (double *)shmat(sc, 0, 0);

    // Initialize A (all 1.0) and B (Identity matrix)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = 1.0;
            B[i * N + j] = (i == j) ? 1.0 : 0.0;
        }
    }
        
    // Spawn K worker processes concurrently
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w + 1) * N / K;

            printf("Worker %d: rows %d to %d\n", w + 1, start, end - 1);

            for (int i = start; i < end; i++) {
                for (int j = 0; j < N; j++) {
                    C[i * N + j] = 0;
                    for (int k = 0; k < N; k++) {
                        C[i * N + j] += A[i * N + k] * B[k * N + j];
                    }
                }
            }
            return 0; // Child exits
        }
    }
    
    // Barrier wait: parent waits for all K workers
    for (int w = 0; w < K; w++) wait(NULL);

    // Verify: C must equal A (all elements 1.0)
    int ok = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (C[i * N + j] != 1.0) ok = 0;
        }
    }

    printf("All workers completed the work\n");
    printf("Verification: %s\n", ok ? "SUCCESS" : "FAILED");

    // Detach and deallocate shared memory segments
    shmdt(A); shmdt(B); shmdt(C);
    shmctl(sa, IPC_RMID, 0);
    shmctl(sb, IPC_RMID, 0);
    shmctl(sc, IPC_RMID, 0);
    return 0;
}
```

---

## How to Compile & Run
```bash
gcc matrix_multi_shm.c -o matrix_multi_shm
./matrix_multi_shm
```
