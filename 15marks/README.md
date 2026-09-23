# 15-Mark Potential Exam Questions & Universal Skeletons

> **Target Question:** Question 2 (15 Marks)  
> **Allocated Time:** ~75–85 minutes  
> **Core Concept:** Multi-process parallel computation using **Shared Memory (POSIX or System V)** + partitioning work across $K$ child processes + barrier synchronization with `wait()` + resource cleanup.  
> **Golden Exam Rule:** *Don't memorize multiple programs.* Memorize **one parallel-fork structure** and simply swap the 3 setup lines and 3 cleanup lines between POSIX and System V!

---

## 🧠 The Universal Master Concept (Read This First!)

In almost every 15-mark question, the structure of the program is **90% identical**:
```
[1. Allocate Shared Memory] ───────► (POSIX: shm_open + ftruncate + mmap) OR (System V: shmget + shmat)
[2. Initialize Data]        ───────► Fill input arrays/matrices
[3. Fork K Workers Loop]    ───────► start = w * N / K; end = (w + 1) * N / K;
                                      Worker computes its slice, then exit(0);
[4. Barrier Wait Loop]      ───────► for (int w = 0; w < K; w++) wait(NULL);
[5. Aggregate / Verify]     ───────► Check correctness
[6. Cleanup Memory]         ───────► (POSIX: munmap + close + shm_unlink) OR (System V: shmdt + shmctl)
```

---

## 🟦 1. Universal POSIX Shared Memory Skeleton

Compile with: `gcc program.c -o program -lrt`

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHM_NAME "/my_shm"

int main() {
    int N, K;
    printf("Enter N and K: ");
    scanf("%d %d", &N, &K);

    int size = N * sizeof(double); // Adjust size as needed

    // 1. Create shared memory object
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); exit(1); }

    // 2. Set size (mandatory after creation)
    ftruncate(fd, size);

    // 3. Map shared memory into process address space
    double *shared = (double *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) { perror("mmap"); exit(1); }

    // 4. Initialize shared data
    for (int i = 0; i < N; i++) {
        shared[i] = 1.0;
    }

    // 5. Create K worker processes
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            // Slicing: calculate assigned slice
            int start = w * N / K;
            int end = (w == K - 1) ? N : (w + 1) * N / K; // Handles uneven division

            printf("Worker %d (PID %d): range %d to %d\n", w + 1, getpid(), start, end - 1);

            // Worker computation on slice [start, end)
            for (int i = start; i < end; i++) {
                // ... compute on shared[i] ...
            }

            // Child must ALWAYS exit
            exit(0);
        }
    }

    // 6. Barrier: Parent waits for ALL K workers
    for (int w = 0; w < K; w++) {
        wait(NULL);
    }

    // 7. Parent reads / verifies results
    printf("All workers finished.\n");

    // 8. Cleanup and destroy shared memory
    munmap(shared, size);
    close(fd);
    shm_unlink(SHM_NAME); // Removes object from /dev/shm

    return 0;
}
```

---

## 🟥 2. Universal System V Shared Memory Skeleton

Compile with: `gcc program.c -o program` (No `-lrt` needed!)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main() {
    int N, K;
    printf("Enter N and K: ");
    scanf("%d %d", &N, &K);

    int size = N * sizeof(double); // Adjust size as needed

    // 1. Create shared memory (IPC_PRIVATE is best for parent-child)
    int shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    // 2. Attach shared memory (Kernel chooses address with NULL)
    double *shared = (double *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) { perror("shmat"); exit(1); }

    // 3. Initialize shared data
    for (int i = 0; i < N; i++) {
        shared[i] = 1.0;
    }

    // 4. Create K worker processes
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            // Slicing: calculate assigned slice
            int start = w * N / K;
            int end = (w == K - 1) ? N : (w + 1) * N / K; // Handles uneven division

            printf("Worker %d (PID %d): range %d to %d\n", w + 1, getpid(), start, end - 1);

            // Worker computation on slice [start, end)
            for (int i = start; i < end; i++) {
                // ... compute on shared[i] ...
            }

            // Child must ALWAYS exit
            shmdt(shared);
            exit(0);
        }
    }

    // 5. Barrier: Parent waits for ALL K workers
    for (int w = 0; w < K; w++) {
        wait(NULL);
    }

    // 6. Parent reads / verifies results
    printf("All workers finished.\n");

    // 7. Cleanup and destroy shared memory
    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL); // Marks segment for eviction from kernel

    return 0;
}
```

---

## ⚡ Side-by-Side Cheat Sheet

| Operation | POSIX API | System V API |
| :--- | :--- | :--- |
| **Header Files** | `<sys/mman.h>`, `<fcntl.h>` | `<sys/shm.h>`, `<sys/ipc.h>` |
| **Identifier** | `int fd` (File Descriptor) | `int shmid` (Shared Memory ID) |
| **Identifier Scope** | Path string (e.g. `"/my_shm"`) | `IPC_PRIVATE` or `ftok()` key |
| **1. Create** | `shm_open(name, O_CREAT \| O_RDWR, 0666)` | `shmget(IPC_PRIVATE, size, IPC_CREAT \| 0666)` |
| **2. Set Size** | `ftruncate(fd, size)` | Sized inside `shmget()` call |
| **3. Attach / Map** | `ptr = mmap(NULL, size, ..., fd, 0)` | `ptr = shmat(shmid, NULL, 0)` |
| **4. Concurrency** | `fork()` loop across $K$ workers | `fork()` loop across $K$ workers |
| **5. Barrier Wait** | `for (w=0; w<K; w++) wait(NULL);` | `for (w=0; w<K; w++) wait(NULL);` |
| **6. Detach** | `munmap(ptr, size)` | `shmdt(ptr)` |
| **7. Close FD** | `close(fd)` | *(Not applicable)* |
| **8. Delete / Evict** | `shm_unlink(name)` | `shmctl(shmid, IPC_RMID, NULL)` |
| **Compiler Flag** | **`-lrt`** (Mandatory) | None needed |

---

## 🚨 6 Critical Pitfalls That Students Miss Out On

### 1. The Sequential `wait()` Bug (Immediate -5 to -10 Marks!)
* **WRONG:** Calling `wait()` inside the worker creation loop:
  ```c
  for (int w = 0; w < K; w++) {
      if (fork() == 0) { ... exit(0); }
      wait(NULL); // ❌ BUG: Parent waits immediately! Workers run sequentially, NOT in parallel!
  }
  ```
* **CORRECT:** Two separate loops:
  ```c
  for (int w = 0; w < K; w++) {
      if (fork() == 0) { ... exit(0); } // Spawn all first
  }
  for (int w = 0; w < K; w++) {
      wait(NULL); // ✅ Barrier wait after all workers are launched
  }
  ```

### 2. Forgetting `exit(0)` in the Worker Child
* If the child finishes its work and does not call `exit(0)`, it will loop back and execute the remaining iterations of the parent's `fork()` loop. This triggers a **fork bomb**!
* **Rule:** The last line of the child block must **always be `exit(0);`**.

### 3. Uneven Workload Partitioning ($N$ not divisible by $K$)
* If $N = 10$ and $K = 3$, simple integer division `w * N / K` to `(w + 1) * N / K` computes:
  * Worker 0: rows $0 \dots 2$ (length 3)
  * Worker 1: rows $3 \dots 5$ (length 3)
  * Worker 2: rows $6 \dots 8$ (length 3) $\rightarrow$ **Row 9 is completely skipped!**
* **The Fix:** Give the remaining elements to the last worker:
  ```c
  int start = w * N / K;
  int end = (w == K - 1) ? N : (w + 1) * N / K;
  ```

### 4. Confusing Identifiers (`fd` vs `shmid`)
* **POSIX:** `shm_open()` returns an `int fd`. You pass `fd` to `mmap(..., fd, 0)`.
* **System V:** `shmget()` returns an `int shmid`. You pass `shmid` to `shmat(shmid, ...)`.

### 5. Forgetting the `-lrt` Flag (POSIX Compilation Error)
* When compiling any program with `shm_open` or `shm_unlink`:
  ```bash
  gcc program.c -o program -lrt
  ```
  If you forget `-lrt`, GCC throws `undefined reference to 'shm_open'`.

### 6. Zombie Shared Memory Leaks
* If a program terminates without `shm_unlink()` or `shmctl(..., IPC_RMID, ...)`, the memory stays resident in the OS.
* **POSIX:** Clean orphaned objects via `rm /dev/shm/*`.
* **System V:** Clean orphaned segments via `ipcrm -m <shmid>`.

---

## 🎯 Question Bank: Priority 1 to 4 (Complete Tested Solutions)

---

### Question 1 (Priority 1): POSIX Distributed Matrix Multiplication ($C = A \times B$)

#### Problem Statement
Write a C program using **POSIX Shared Memory** to perform parallel matrix multiplication $C = A \times B$ of two $N \times N$ matrices using $K$ worker child processes:
1. Create 3 shared memory objects (`/matrix_A`, `/matrix_B`, `/matrix_C`), set size, and map with `PROT_READ | PROT_WRITE` and `MAP_SHARED`.
2. Initialize $A$ with `1.0` and $B$ as an Identity matrix ($B[i][j] = 1.0$ if $i == j$ else $0.0$).
3. Fork $K$ workers to compute non-overlapping row slices in parallel.
4. Parent waits for all $K$ workers, verifies that every element in $C$ equals `1.0` ($A \times I = A$), and unlinks all objects.

#### Solution Code (`posix_matrix_multi.c`)
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

    // 2. Set size
    ftruncate(fdA, size);
    ftruncate(fdB, size);
    ftruncate(fdC, size);

    // 3. Map shared memory
    double *A = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdA, 0);
    double *B = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdB, 0);
    double *C = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fdC, 0);

    // 4. Initialize A and B
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = 1.0;
            B[i * N + j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // 5. Create K workers
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w == K - 1) ? N : (w + 1) * N / K;

            printf("Worker %d: rows %d to %d\n", w + 1, start, end - 1);

            for (int i = start; i < end; i++) {
                for (int j = 0; j < N; j++) {
                    C[i * N + j] = 0;
                    for (int k = 0; k < N; k++) {
                        C[i * N + j] += A[i * N + k] * B[k * N + j];
                    }
                }
            }

            exit(0);
        }
    }

    // 6. Barrier: Parent waits for all workers
    for (int w = 0; w < K; w++)
        wait(NULL);

    // 7. Verify result
    int ok = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (C[i * N + j] != 1.0)
                ok = 0;
        }
    }

    printf("All workers completed the work\n");
    printf("Verification: %s\n", ok ? "SUCCESS" : "FAILED");

    // 8. Cleanup
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

#### Compile & Run
```bash
gcc posix_matrix_multi.c -o posix_matrix_multi -lrt
./posix_matrix_multi
```

---

### Question 2 (Priority 2): System V Distributed Matrix Multiplication (Week 4 Lab 3)

#### Problem Statement
Same problem as Question 1, but implemented using the **System V Shared Memory API** (`shmget`, `shmat`, `shmdt`, `shmctl`).

#### Solution Code (`sysv_matrix_multi.c`)
```c
#include <stdio.h>
#include <stdlib.h>
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

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = 1.0;
            B[i * N + j] = (i == j) ? 1.0 : 0.0;
        }
    }
        
    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w == K - 1) ? N : (w + 1) * N / K;

            printf("Worker %d: rows %d to %d\n", w + 1, start, end - 1);

            for (int i = start; i < end; i++) {
                for (int j = 0; j < N; j++) {
                    C[i * N + j] = 0;
                    for (int k = 0; k < N; k++) {
                        C[i * N + j] += A[i * N + k] * B[k * N + j];
                    }
                }
            }
            shmdt(A); shmdt(B); shmdt(C);
            exit(0);
        }
    }
    
    for (int w = 0; w < K; w++) wait(NULL);

    int ok = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (C[i * N + j] != 1.0) ok = 0;
        }
    }

    printf("All workers completed the work\n");
    printf("Verification: %s\n", ok ? "SUCCESS" : "FAILED");

    shmdt(A); shmdt(B); shmdt(C);
    shmctl(sa, IPC_RMID, 0);
    shmctl(sb, IPC_RMID, 0);
    shmctl(sc, IPC_RMID, 0);
    return 0;
}
```

#### Compile & Run
```bash
gcc sysv_matrix_multi.c -o sysv_matrix_multi
./sysv_matrix_multi
```

---

### Question 3 (Priority 3): POSIX Parallel Array Sum / Reduction

#### Problem Statement
Compute the sum of an array of $N$ integers using $K$ workers and POSIX Shared Memory. Each worker sums its assigned slice into a shared `partial_sums` array, and the parent aggregates the total and verifies against $N(N+1)/2$.

#### Solution Code (`posix_array_sum.c`)
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

    int fd_data = shm_open(SHM_DATA, O_CREAT | O_RDWR, 0666);
    int fd_sums = shm_open(SHM_SUMS, O_CREAT | O_RDWR, 0666);
    ftruncate(fd_data, size_data);
    ftruncate(fd_sums, size_sums);

    int *data = mmap(NULL, size_data, PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);
    long long *partial_sums = mmap(NULL, size_sums, PROT_READ | PROT_WRITE, MAP_SHARED, fd_sums, 0);

    for (int i = 0; i < N; i++) {
        data[i] = i + 1;
    }

    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w == K - 1) ? N : (w + 1) * N / K;

            long long local_sum = 0;
            for (int i = start; i < end; i++) {
                local_sum += data[i];
            }

            partial_sums[w] = local_sum;
            exit(0);
        }
    }

    for (int w = 0; w < K; w++) wait(NULL);

    long long total_sum = 0;
    for (int w = 0; w < K; w++) {
        total_sum += partial_sums[w];
    }

    long long expected = (long long)N * (N + 1) / 2;
    printf("Computed Parallel Sum : %lld\n", total_sum);
    printf("Expected Sum (Formula): %lld\n", expected);
    printf("Verification          : %s\n", (total_sum == expected) ? "SUCCESS" : "FAILED");

    munmap(data, size_data);
    munmap(partial_sums, size_sums);
    close(fd_data);
    close(fd_sums);
    shm_unlink(SHM_DATA);
    shm_unlink(SHM_SUMS);

    return 0;
}
```

#### Compile & Run
```bash
gcc posix_array_sum.c -o posix_array_sum -lrt
./posix_array_sum
```

---

### Question 4 (Priority 4): POSIX Parallel Matrix Addition ($C = A + B$)

#### Problem Statement
Add two $N \times N$ matrices ($C[i][j] = A[i][j] + B[i][j]$) using $K$ worker child processes and POSIX Shared Memory.

#### Solution Code (`posix_matrix_add.c`)
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

    for (int i = 0; i < N * N; i++) {
        A[i] = 2;
        B[i] = 3;
    }

    for (int w = 0; w < K; w++) {
        if (fork() == 0) {
            int start = w * N / K;
            int end = (w == K - 1) ? N : (w + 1) * N / K;

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

#### Compile & Run
```bash
gcc posix_matrix_add.c -o posix_matrix_add -lrt
./posix_matrix_add
```
