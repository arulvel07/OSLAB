# 15-Mark Potential Exam Questions & Complete Solutions

> **Target Question:** Question 2 (15 Marks)  
> **Allocated Time:** ~75–85 minutes  
> **Key Skills Tested:** Multi-process parallel computation, shared memory allocation (System V & POSIX), workload slicing across $K$ worker processes, lock-free non-overlapping memory writes, barrier synchronization via `wait()`, output correctness verification, and kernel memory cleanup.

---

## Priority Ranking of 15-Mark Topics

| Priority | Topic / Concept | Probability | Why Examiners Choose It |
| :---: | :--- | :---: | :--- |
| **P1** | **Distributed Matrix Multiplication ($A \times B$) via Shared Memory** | **85%–90%** | The crown jewel of Week 4 labs. Tests shared memory allocation, 2D index flattening, row slicing, barrier waits, and verification. |
| **P2** | **Distributed Parallel Array Reduction (Sum & Global Maximum)** | **80%** | Standard parallel computing problem. Workers write partial sums/maxima to a shared results array, followed by parent aggregation. |
| **P3** | **Parallel Matrix Processing using POSIX Shared Memory (`-lrt`)** | **70%** | Same parallel slicing paradigm, but tests modern POSIX API (`shm_open`, `ftruncate`, `mmap`, `shm_unlink`). |
| **P4** | **Shared Memory Producer-Consumer with Flag Handshake** | **55%** | Tests structured shared data (`struct`), status flags, memory synchronization without mutexes, and clean shutdown. |

---

## Question 1 (Priority 1): Distributed Matrix Multiplication ($C = A \times B$) Using $K$ Worker Processes

### Problem Statement
Given two large $N \times N$ matrices $A$ and $B$, write a multi-process C program using System V Shared Memory to compute the product matrix $C = A \times B$ concurrently using $K$ worker child processes ($K \le N$):
1. **Shared Memory Allocation:** The parent allocates shared memory for matrices $A$, $B$, and $C$ using `shmget(IPC_PRIVATE, ...)` and maps them via `shmat()`.
2. **Matrix Initialization:**
   - Initialize $A$ with all elements set to `1.0`.
   - Initialize $B$ as an Identity matrix ($B[i][j] = 1.0$ if $i == j$, else $0.0$).
3. **Parallel Computation:**
   - The parent forks $K$ worker child processes.
   - Each worker $w$ ($0 \le w < K$) computes a mutually exclusive slice of rows from `start_row` to `end_row - 1`.
   - Worker computes $C[i][j] = \sum_{k=0}^{N-1} A[i][k] \cdot B[k][j]$.
4. **Lock-Free Concurrency Explanation:** Explain why no mutexes or semaphores are required during the matrix multiplication.
5. **Barrier Synchronization & Verification:**
   - The parent waits for all $K$ workers to terminate.
   - Verify that $C = A \times I = A$ (every element in $C$ must evaluate to `1.0`). Print `Verification: SUCCESS` or `FAILED`.
6. **Cleanup:** Detach all shared memory segments (`shmdt`) and destroy them from the kernel registry (`shmctl` with `IPC_RMID`).

---

### Core Concept: Why No Mutexes are Needed
Because each worker $w$ computes only rows in $[start, end)$ of Matrix $C$:
* Workers **only read** from Matrices $A$ and $B$ (concurrent reads are completely safe).
* Workers **write exclusively** to their assigned row partitions of Matrix $C$.
* There is **zero memory overlap** between the write locations of any two workers. Hence, no critical section or race condition exists!

---

### Complete Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main() {
    int N, K;
    printf("Enter matrix dimension N: ");
    if (scanf("%d", &N) != 1 || N <= 0) {
        printf("Invalid dimension.\n");
        return 1;
    }

    printf("Enter number of worker processes K (1 <= K <= %d): ", N);
    if (scanf("%d", &K) != 1 || K <= 0 || K > N) {
        printf("Invalid number of workers.\n");
        return 1;
    }

    // 1. Allocate shared memory for 3 matrices of size N x N
    size_t matrix_bytes = N * N * sizeof(double);
    int sa = shmget(IPC_PRIVATE, matrix_bytes, IPC_CREAT | 0666);
    int sb = shmget(IPC_PRIVATE, matrix_bytes, IPC_CREAT | 0666);
    int sc = shmget(IPC_PRIVATE, matrix_bytes, IPC_CREAT | 0666);

    if (sa < 0 || sb < 0 || sc < 0) {
        perror("shmget failed");
        return 1;
    }

    // 2. Attach shared memory segments
    double *A = (double *)shmat(sa, NULL, 0);
    double *B = (double *)shmat(sb, NULL, 0);
    double *C = (double *)shmat(sc, NULL, 0);

    if (A == (void *)-1 || B == (void *)-1 || C == (void *)-1) {
        perror("shmat failed");
        return 1;
    }

    // 3. Initialize Matrix A (all 1.0) and Matrix B (Identity Matrix)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = 1.0;
            B[i * N + j] = (i == j) ? 1.0 : 0.0;
            C[i * N + j] = 0.0; // Clear C
        }
    }

    printf("\nParent: Initialized matrices. Spawning %d workers in parallel...\n", K);

    // 4. Fork K worker processes concurrently
    for (int w = 0; w < K; w++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // === CHILD WORKER PROCESS ===
            // Workload partitioning: handles uneven N / K division gracefully
            int start_row = w * N / K;
            int end_row = (w == K - 1) ? N : (w + 1) * N / K;

            printf("[Worker %d | PID %d] Processing rows %d to %d\n", 
                   w + 1, getpid(), start_row, end_row - 1);

            for (int i = start_row; i < end_row; i++) {
                for (int j = 0; j < N; j++) {
                    double sum = 0.0;
                    for (int k = 0; k < N; k++) {
                        sum += A[i * N + k] * B[k * N + j];
                    }
                    C[i * N + j] = sum;
                }
            }

            // Detach memory in child before exiting
            shmdt(A);
            shmdt(B);
            shmdt(C);
            exit(0);
        }
    }

    // 5. Barrier Synchronization: Parent waits for all K workers to complete
    for (int w = 0; w < K; w++) {
        wait(NULL);
    }
    printf("\nParent: All %d worker processes have completed execution.\n", K);

    // 6. Verification: Since B is Identity, C = A * I = A (every element should be 1.0)
    int verification_passed = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (C[i * N + j] != 1.0) {
                verification_passed = 0;
                break;
            }
        }
    }

    printf("Verification Result: %s\n", verification_passed ? "SUCCESS (C == A)" : "FAILED");

    // 7. Cleanup & Eviction of Kernel Shared Memory
    shmdt(A);
    shmdt(B);
    shmdt(C);

    shmctl(sa, IPC_RMID, NULL);
    shmctl(sb, IPC_RMID, NULL);
    shmctl(sc, IPC_RMID, NULL);
    printf("Kernel shared memory segments successfully removed.\n");

    return 0;
}
```

### Compilation & Verification
```bash
gcc matrix_multi_shm.c -o matrix_multi_shm
./matrix_multi_shm
```

---

## Question 2 (Priority 2): Parallel Array Reduction (Sum & Global Maximum) via Shared Memory

### Problem Statement
Write a multi-process C program to compute the **sum** and the **global maximum** of an array of $N$ integers using $K$ worker child processes:
1. The parent allocates shared memory for:
   - An array `data` of $N$ integers ($N = 10,000$ or user input).
   - Two result arrays of size $K$: `partial_sums` and `partial_maxs`.
2. The parent populates `data` with values.
3. The parent spawns $K$ worker children. Worker $w$ computes the sum and maximum of its assigned range $[start, end)$ and writes them into `partial_sums[w]` and `partial_maxs[w]`.
4. The parent waits for all $K$ workers to terminate.
5. The parent combines the $K$ partial sums and finds the largest of the $K$ partial maxima to obtain the overall answers.
6. Verify against sequential execution and destroy shared memory segments.

---

### Complete Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define DEFAULT_N 10000

int main() {
    int N = DEFAULT_N;
    int K;

    printf("Enter number of worker processes K (e.g. 4): ");
    if (scanf("%d", &K) != 1 || K <= 0) return 1;

    // Allocate shared memory
    int shm_data = shmget(IPC_PRIVATE, N * sizeof(int), IPC_CREAT | 0666);
    int shm_sums = shmget(IPC_PRIVATE, K * sizeof(long long), IPC_CREAT | 0666);
    int shm_maxs = shmget(IPC_PRIVATE, K * sizeof(int), IPC_CREAT | 0666);

    if (shm_data < 0 || shm_sums < 0 || shm_maxs < 0) {
        perror("shmget");
        return 1;
    }

    int *data = (int *)shmat(shm_data, NULL, 0);
    long long *partial_sums = (long long *)shmat(shm_sums, NULL, 0);
    int *partial_maxs = (int *)shmat(shm_maxs, NULL, 0);

    // Initialize array with predictable formula: data[i] = (i % 100) + 1
    // (Known sum and max for easy verification)
    long long expected_sum = 0;
    int expected_max = -1;
    for (int i = 0; i < N; i++) {
        data[i] = (i % 100) + 1;
        expected_sum += data[i];
        if (data[i] > expected_max) expected_max = data[i];
    }

    // Spawn K workers concurrently
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w == K - 1) ? N : (w + 1) * N / K;

            long long local_sum = 0;
            int local_max = data[start];

            for (int i = start; i < end; i++) {
                local_sum += data[i];
                if (data[i] > local_max) local_max = data[i];
            }

            partial_sums[w] = local_sum;
            partial_maxs[w] = local_max;

            shmdt(data); shmdt(partial_sums); shmdt(partial_maxs);
            exit(0);
        }
    }

    // Barrier wait
    for (int w = 0; w < K; w++) wait(NULL);

    // Parent aggregates partial results from the K workers
    long long total_sum = 0;
    int global_max = partial_maxs[0];
    for (int w = 0; w < K; w++) {
        total_sum += partial_sums[w];
        if (partial_maxs[w] > global_max) global_max = partial_maxs[w];
    }

    printf("\n=== Parallel Reduction Results ===\n");
    printf("Parallel Sum : %lld (Expected: %lld) -> %s\n", 
           total_sum, expected_sum, (total_sum == expected_sum) ? "MATCH ✅" : "ERROR ❌");
    printf("Global Max   : %d (Expected: %d) -> %s\n", 
           global_max, expected_max, (global_max == expected_max) ? "MATCH ✅" : "ERROR ❌");

    // Cleanup
    shmdt(data); shmdt(partial_sums); shmdt(partial_maxs);
    shmctl(shm_data, IPC_RMID, NULL);
    shmctl(shm_sums, IPC_RMID, NULL);
    shmctl(shm_maxs, IPC_RMID, NULL);

    return 0;
}
```

### Compilation & Verification
```bash
gcc array_reduce_shm.c -o array_reduce_shm
./array_reduce_shm
```

---

## Question 3 (Priority 3): Parallel Matrix Transposition using POSIX Shared Memory API (`-lrt`)

### Problem Statement
Write a multi-process C program that transposes an $N \times N$ matrix $A$ into matrix $T$ using $K$ worker child processes and **POSIX Shared Memory** (`shm_open`, `ftruncate`, `mmap`, `munmap`, `shm_unlink`):
1. Create a named POSIX shared memory object `/shm_matrix_transpose`.
2. Truncate its size to hold both Matrix $A$ and Matrix $T$ ($2 \times N \times N \times \text{sizeof(int)}$).
3. Map the memory using `PROT_READ | PROT_WRITE` and `MAP_SHARED`.
4. Parent initializes Matrix $A$ with $A[i][j] = i \cdot N + j$.
5. Parent forks $K$ workers. Worker $w$ transposes its assigned row slice such that $T[j][i] = A[i][j]$.
6. Parent waits for all workers, verifies that $T[j][i] == A[i][j]$, unmaps memory, and deletes the object via `shm_unlink()`.

---

### Complete Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SHM_NAME "/shm_matrix_transpose"

int main() {
    int N = 6; // Matrix size 6x6
    int K = 3; // 3 workers

    size_t matrix_size = N * N * sizeof(int);
    size_t total_size = 2 * matrix_size; // Space for both A and T

    // 1. Open POSIX shared memory object
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); return 1; }

    // 2. Set size
    if (ftruncate(fd, total_size) == -1) { perror("ftruncate"); return 1; }

    // 3. Map memory
    int *base = (int *)mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED) { perror("mmap"); return 1; }

    int *A = base;
    int *T = base + (N * N);

    // Initialize Matrix A
    printf("Original Matrix A:\n");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = i * 10 + j;
            printf("%3d ", A[i * N + j]);
        }
        printf("\n");
    }

    // 4. Fork K workers for parallel transposition
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start_row = w * N / K;
            int end_row = (w == K - 1) ? N : (w + 1) * N / K;

            for (int i = start_row; i < end_row; i++) {
                for (int j = 0; j < N; j++) {
                    T[j * N + i] = A[i * N + j]; // Transpose write
                }
            }
            munmap(base, total_size);
            close(fd);
            exit(0);
        }
    }

    // 5. Barrier wait
    for (int w = 0; w < K; w++) wait(NULL);

    // Print and verify transposed matrix
    printf("\nTransposed Matrix T:\n");
    int correct = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%3d ", T[i * N + j]);
            if (T[i * N + j] != A[j * N + i]) correct = 0;
        }
        printf("\n");
    }

    printf("\nVerification: %s\n", correct ? "SUCCESS ✅" : "FAILED ❌");

    // 6. Cleanup
    munmap(base, total_size);
    close(fd);
    shm_unlink(SHM_NAME); // Remove from /dev/shm
    return 0;
}
```

### Compilation & Verification
```bash
gcc matrix_posix_transpose.c -o matrix_posix_transpose -lrt
./matrix_posix_transpose
```

---

## Question 4 (Priority 4): Synchronized Producer-Consumer via Shared Memory Flag Handshake

### Problem Statement
Write two processes (parent and child) that communicate through a shared memory buffer using a lock-free **status flag handshake protocol**:
1. The shared structure contains:
   ```c
   struct shared_channel {
       int ready;      // 0: empty, 1: data ready to consume, -1: terminate
       int data;       // integer data
   };
   ```
2. The parent (Producer) writes numbers $1$ to $10$ one by one into `data`. After writing an item, it sets `ready = 1` and waits until the child sets `ready = 0`.
3. The child (Consumer) waits until `ready == 1`, prints the received value, and resets `ready = 0` as an acknowledgment.
4. After sending all $10$ items, the parent sets `ready = -1` to signal shutdown, waits for child exit, and destroys the shared memory segment.

---

### Complete Solution Code
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

struct shared_channel {
    volatile int ready; // volatile prevents compiler optimization from caching the flag
    int data;
};

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(struct shared_channel), IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); return 1; }

    struct shared_channel *channel = (struct shared_channel *)shmat(shmid, NULL, 0);
    channel->ready = 0;

    pid_t pid = fork();

    if (pid == 0) {
        // === CONSUMER (CHILD) ===
        while (1) {
            // Wait for producer to set ready flag
            while (channel->ready == 0) {
                usleep(100); // Small sleep to avoid burning 100% CPU
            }

            if (channel->ready == -1) {
                // Termination signal received
                printf("[Consumer Child] Received termination signal. Exiting...\n");
                break;
            }

            printf("[Consumer Child] Read data: %d\n", channel->data);

            // Acknowledge read
            channel->ready = 0;
        }

        shmdt(channel);
        exit(0);
    } else {
        // === PRODUCER (PARENT) ===
        for (int i = 1; i <= 10; i++) {
            // Wait until previous data has been acknowledged by consumer
            while (channel->ready != 0) {
                usleep(100);
            }

            channel->data = i * 10;
            printf("[Producer Parent] Sent data: %d\n", channel->data);
            channel->ready = 1; // Signal consumer
        }

        // Wait for last item to be read
        while (channel->ready != 0) {
            usleep(100);
        }

        // Send termination sentinel
        channel->ready = -1;

        wait(NULL); // Reap child

        // Cleanup
        shmdt(channel);
        shmctl(shmid, IPC_RMID, NULL);
        printf("[Producer Parent] Shared memory released cleanly.\n");
    }

    return 0;
}
```

### Compilation & Verification
```bash
gcc shm_handshake.c -o shm_handshake
./shm_handshake
```
