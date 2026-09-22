# 15-Mark Potential Exam Questions & Complete Solutions

> **Target Question:** Question 2 (15 Marks)  
> **Allocated Time:** ~75–85 minutes  
> **Core Concept:** Multi-process parallel computation using **Shared Memory (POSIX / System V)** + dividing work across $K$ child processes + barrier synchronization with `wait()` + cleanup.  
> **Compiler Reminder:** Always compile POSIX programs with **`-lrt`**!

---

## Priority Ranking of 15-Mark Topics

| Priority | Topic / Concept | Probability | API Used | Key Highlights |
| :---: | :--- | :---: | :---: | :--- |
| **P1** | **Distributed Matrix Multiplication ($A \times B$)** | **90%** | **POSIX** (`shm_open`, `mmap`) | The top candidate. Allocates matrices in POSIX shared memory, splits rows across $K$ workers, verifies $C = A \times I = A$. |
| **P2** | **Distributed Matrix Multiplication (System V Alternative)** | **85%** | **System V** (`shmget`, `shmat`) | Exact same problem implemented using legacy System V (`IPC_PRIVATE`), exactly like Week 4 Q3 lab. |
| **P3** | **Parallel Array Sum / Reduction** | **75%** | **POSIX** (`shm_open`, `mmap`) | $N$ integers in shared memory. $K$ workers compute partial sums into a shared result array. Parent calculates total. |
| **P4** | **Parallel Matrix Addition ($C = A + B$)** | **65%** | **POSIX** (`shm_open`, `mmap`) | Simpler arithmetic variation of matrix multiplication across $K$ workers. |

---

## Question 1 (Priority 1): POSIX Distributed Matrix Multiplication ($C = A \times B$)

### Problem Statement
Write a C program using **POSIX Shared Memory** (`shm_open`, `ftruncate`, `mmap`, `munmap`, `shm_unlink`) to perform parallel matrix multiplication $C = A \times B$ of two $N \times N$ matrices using $K$ worker child processes ($K \le N$):
1. **Allocate & Map:** Create 3 POSIX shared memory objects (`/matrix_A`, `/matrix_B`, `/matrix_C`), set size to $N \times N \times \text{sizeof(double)}$, and map with `PROT_READ | PROT_WRITE` and `MAP_SHARED`.
2. **Initialize:** Fill $A$ with all `1.0` and $B$ as an Identity matrix ($B[i][j] = 1.0$ if $i == j$ else $0.0$).
3. **Partition & Fork:** Fork $K$ worker child processes. Each worker $w$ computes its assigned row slice from `start = w * N / K` to `end = (w + 1) * N / K`.
4. **Barrier Wait & Verification:** Parent waits for all $K$ workers using `wait(NULL)`, verifies that every element in $C$ equals `1.0` ($A \times I = A$), and unlinks all shared memory objects.

---

### Solution Code (`posix_matrix_multi.c`)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHM_A "/matrix_A"
#define SHM_B "/matrix_B"
#define SHM_C "/matrix_C"

int main() {
    int N, K;

    printf("Enter value of N: ");
    scanf("%d", &N);

    printf("Enter number of workers (K <= %d): ", N);
    scanf("%d", &K);

    int size = N * N * sizeof(double);

    // 1. Create shared memory objects
    int fdA = shm_open(SHM_A, O_CREAT | O_RDWR, 0666);
    int fdB = shm_open(SHM_B, O_CREAT | O_RDWR, 0666);
    int fdC = shm_open(SHM_C, O_CREAT | O_RDWR, 0666);

    // 2. Set storage capacity
    ftruncate(fdA, size);
    ftruncate(fdB, size);
    ftruncate(fdC, size);

    // 3. Map shared memory into process address space
    double *A = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdA, 0);
    double *B = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdB, 0);
    double *C = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdC, 0);

    // 4. Initialize A (all 1.0) and B (Identity matrix)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = 1.0;
            B[i * N + j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // 5. Create K worker processes
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w + 1) * N / K;

            printf("Worker %d: rows %d to %d\n", w + 1, start, end - 1);

            // Compute assigned row partition
            for (int i = start; i < end; i++) {
                for (int j = 0; j < N; j++) {
                    C[i * N + j] = 0;
                    for (int k = 0; k < N; k++) {
                        C[i * N + j] += A[i * N + k] * B[k * N + j];
                    }
                }
            }

            exit(0); // Worker terminates
        }
    }

    // 6. Barrier: Parent waits for all K workers
    for (int w = 0; w < K; w++)
        wait(NULL);

    // 7. Verify result (Since B is Identity, C must equal A)
    int ok = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (C[i * N + j] != 1.0)
                ok = 0;
        }
    }

    printf("All workers completed the work\n");
    printf("Verification: %s\n", ok ? "SUCCESS" : "FAILED");

    // 8. Cleanup and destroy shared memory
    munmap(A, size);
    munmap(B, size);
    munmap(C, size);

    close(fdA);
    close(fdB);
    close(fdC);

    shm_unlink(SHM_A);
    shm_unlink(SHM_B);
    shm_unlink(SHM_C);

    return 0;
}
```

### How to Compile & Run
```bash
gcc posix_matrix_multi.c -o posix_matrix_multi -lrt
./posix_matrix_multi
```

---

## Question 2 (Priority 2): System V Distributed Matrix Multiplication (Week 4 Lab 3)

### Problem Statement
If the exam specifically asks to solve the problem using **System V Shared Memory** (`shmget`, `shmat`, `shmdt`, `shmctl`) instead of POSIX:

### Solution Code (`sysv_matrix_multi.c`)
```c
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main() {
    int N, K;
    printf("Enter value of N:- ");
    scanf("%d", &N);
    
    printf("Enter no of workers(K<=%d):- ", N);
    scanf("%d", &K);
    
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

### How to Compile & Run
```bash
gcc sysv_matrix_multi.c -o sysv_matrix_multi
./sysv_matrix_multi
```

---

## Question 3 (Priority 3): POSIX Parallel Array Sum / Reduction

### Problem Statement
Write a C program using **POSIX Shared Memory** to compute the sum of an array of $N$ integers using $K$ worker child processes:
1. Allocate shared memory for the data array `data` ($N$ integers) and a results array `partial_sums` ($K$ integers).
2. Initialize `data[i] = i + 1`.
3. Fork $K$ workers. Worker $w$ computes the sum of elements from index `start = w * N / K` to `end = (w + 1) * N / K` and writes the sum into `partial_sums[w]`.
4. Parent waits for all workers, adds up `partial_sums[0]` through `partial_sums[K-1]`, and compares it with the expected formula $N(N + 1) / 2$.

---

### Solution Code (`posix_array_sum.c`)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHM_DATA "/arr_data"
#define SHM_SUMS "/arr_sums"

int main() {
    int N, K;
    printf("Enter array size N: ");
    scanf("%d", &N);
    printf("Enter number of workers K: ");
    scanf("%d", &K);

    int size_data = N * sizeof(int);
    int size_sums = K * sizeof(long long);

    // 1. Create and size shared memory
    int fd_data = shm_open(SHM_DATA, O_CREAT | O_RDWR, 0666);
    int fd_sums = shm_open(SHM_SUMS, O_CREAT | O_RDWR, 0666);
    ftruncate(fd_data, size_data);
    ftruncate(fd_sums, size_sums);

    // 2. Map memory
    int *data = mmap(NULL, size_data, PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);
    long long *partial_sums = mmap(NULL, size_sums, PROT_READ | PROT_WRITE, MAP_SHARED, fd_sums, 0);

    // 3. Initialize array: data[i] = i + 1
    for (int i = 0; i < N; i++) {
        data[i] = i + 1;
    }

    // 4. Fork K workers
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w + 1) * N / K;

            long long local_sum = 0;
            for (int i = start; i < end; i++) {
                local_sum += data[i];
            }

            partial_sums[w] = local_sum;
            exit(0);
        }
    }

    // 5. Barrier wait
    for (int w = 0; w < K; w++) wait(NULL);

    // 6. Parent aggregates partial sums
    long long total_sum = 0;
    for (int w = 0; w < K; w++) {
        total_sum += partial_sums[w];
    }

    long long expected = (long long)N * (N + 1) / 2;
    printf("Computed Parallel Sum : %lld\n", total_sum);
    printf("Expected Sum (Formula): %lld\n", expected);
    printf("Verification          : %s\n", (total_sum == expected) ? "SUCCESS" : "FAILED");

    // 7. Cleanup
    munmap(data, size_data);
    munmap(partial_sums, size_sums);
    close(fd_data);
    close(fd_sums);
    shm_unlink(SHM_DATA);
    shm_unlink(SHM_SUMS);

    return 0;
}
```

### How to Compile & Run
```bash
gcc posix_array_sum.c -o posix_array_sum -lrt
./posix_array_sum
```

---

## Question 4 (Priority 4): POSIX Parallel Matrix Addition ($C = A + B$)

### Problem Statement
Write a C program using POSIX Shared Memory to add two $N \times N$ matrices ($C[i][j] = A[i][j] + B[i][j]$) using $K$ worker processes.

### Solution Code (`posix_matrix_add.c`)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHM_ADD_A "/matrix_add_A"
#define SHM_ADD_B "/matrix_add_B"
#define SHM_ADD_C "/matrix_add_C"

int main() {
    int N = 4, K = 2;
    int size = N * N * sizeof(int);

    int fdA = shm_open(SHM_ADD_A, O_CREAT | O_RDWR, 0666);
    int fdB = shm_open(SHM_ADD_B, O_CREAT | O_RDWR, 0666);
    int fdC = shm_open(SHM_ADD_C, O_CREAT | O_RDWR, 0666);

    ftruncate(fdA, size);
    ftruncate(fdB, size);
    ftruncate(fdC, size);

    int *A = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdA, 0);
    int *B = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdB, 0);
    int *C = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdC, 0);

    // Initialize: A has 2s, B has 3s -> C should have 5s
    for (int i = 0; i < N * N; i++) {
        A[i] = 2;
        B[i] = 3;
    }

    // Fork K workers
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w + 1) * N / K;

            for (int i = start; i < end; i++) {
                for (int j = 0; j < N; j++) {
                    C[i * N + j] = A[i * N + j] + B[i * N + j];
                }
            }
            exit(0);
        }
    }

    for (int w = 0; w < K; w++) wait(NULL);

    int ok = 1;
    for (int i = 0; i < N * N; i++) {
        if (C[i] != 5) ok = 0;
    }

    printf("Matrix Addition Verification: %s (all elements == 5)\n", ok ? "SUCCESS" : "FAILED");

    munmap(A, size); munmap(B, size); munmap(C, size);
    close(fdA); close(fdB); close(fdC);
    shm_unlink(SHM_ADD_A); shm_unlink(SHM_ADD_B); shm_unlink(SHM_ADD_C);

    return 0;
}
```

### How to Compile & Run
```bash
gcc posix_matrix_add.c -o posix_matrix_add -lrt
./posix_matrix_add
```
