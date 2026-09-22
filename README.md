# Operating Systems Lab - Complete Repository & Exam Preparation Guide

> **Exam Schedule & Format:**
> - **Duration:** 2 Hours 30 Minutes
> - **Total Marks:** 25 Marks
> - **Question 1:** 10 Marks
> - **Question 2:** 15 Marks
> - **Syllabus Constraint:** POSIX / Linux System Calls & concepts discussed in class; **No xv6 questions**.

---

## 📂 Lab Directory & Question Index

| Lab | Question | Topic / Concept | System Calls / Tools | Link |
| :--- | :--- | :--- | :--- | :--- |
| **Week 1** | **Q1** | Text File Word/Char/Line Counter & Longest Line | C++ `ifstream`, `stringstream` | [Week 1 Q1](./week1/q1/README.md) |
| | **Q2** | Create 8 Processes with 3 `fork()` Calls | `fork()`, `getpid()` | [Week 1 Q2](./week1/q2/README.md) |
| | **Q3** | Create <16 Processes with 4 `fork()` Calls | `fork()`, `getpid()` | [Week 1 Q3](./week1/q3/README.md) |
| | **Q4** | Zombie Process Creation & Reaping | `fork()`, `exit()`, `wait()`, `ps` | [Week 1 Q4](./week1/q4/README.md) |
| | **Q5** | Exact 4 Processes Tree with 3 `fork()` Calls | `fork()`, `getpid()`, `getppid()` | [Week 1 Q5](./week1/q5/README.md) |
| | **Q6** | Linux CLI: Folders, File Creation, `grep`, `cat` | Bash CLI (`mkdir`, `grep`, `cat`) | [Week 1 Q6](./week1/q6/README.md) |
| **Week 2** | **Q1** | Log Analysis Pipeline for 401/403 Offending IPs | `grep`, `sort`, `uniq`, `head` | [Week 2 Q1](./week2/q1/README.md) |
| | **Q2** | Child Process File Backup Utility (`cp`) | `fork()`, `execlp()`, `waitpid()` | [Week 2 Q2](./week2/q2/README.md) |
| | **Q3** | Standalone Worker Controller (Parallel `calc_worker`) | `fork()`, `execl()`, `wait()` | [Week 2 Q3](./week2/q3/README.md) |
| | **Q4** | Orphan Process Detection without `sleep()` | `fork()`, `getppid()`, busy wait | [Week 2 Q4](./week2/q4/README.md) |
| **Week 3** | **Q1** | Unidirectional Anonymous Pipe IPC ("Hello, Child!") | `pipe()`, `read()`, `write()`, `close()` | [Week 3 Q1](./week3/q1/README.md) |
| | **Q2** | Bidirectional Pipe Calculator with Error Handling | `pipe()` (x2), `fork()`, `wait()` | [Week 3 Q2](./week3/q2/README.md) |
| **Week 4** | **Q1** | POSIX Shared Memory String Broadcaster | `shm_open()`, `mmap()`, `shm_unlink()` | [Week 4 Q1](./week4/q1/README.md) |
| | **Q2** | System V Shared Memory Registry & `ipcs` Inspection | `ftok()`, `shmget()`, `shmat()`, `shmctl()` | [Week 4 Q2](./week4/q2/README.md) |
| | **Q3** | Distributed Parallel Matrix Multiplication | `shmget()`, `shmat()`, `fork()` worker pool | [Week 4 Q3](./week4/q3/README.md) |

---

# Comprehensive Preparation Guide

## Table of Contents
1. [Topic Overview by Lab Week](#1-topic-overview-by-lab-week)
2. [Priority Ranking: Concepts & Exam Weightage](#2-priority-ranking-concepts--exam-weightage)
3. [Essential Code Templates & Complete Solutions](#3-essential-code-templates--complete-solutions)
   - [3.1 Bidirectional Pipes: Calculator Worker](#31-bidirectional-pipes-calculator-worker)
   - [3.2 POSIX Shared Memory (`shm_open`, `mmap`, `-lrt`)](#32-posix-shared-memory-shm_open-mmap--lrt)
   - [3.3 System V Shared Memory (`ftok`, `shmget`, `shmat`)](#33-system-v-shared-memory-ftok-shmget-shmat)
   - [3.4 Parallel Matrix Multiplication with Disjoint Memory Slices](#34-parallel-matrix-multiplication-with-disjoint-memory-slices)
   - [3.5 Process Orchestration & Exec (`fork` + `execlp` + `waitpid`)](#35-process-orchestration--exec-fork--execlp--waitpid)
   - [3.6 Zombie and Orphan Creation & Detection](#36-zombie-and-orphan-creation--detection)
4. [Predicted Exam Questions (10 Marks vs 15 Marks)](#4-predicted-exam-questions-10-marks-vs-15-marks)
5. [Critical Gotchas & Compilation Cheat Sheet](#5-critical-gotchas--compilation-cheat-sheet)

---

## 1. Topic Overview by Lab Week

### **Week 1: Process Creation, Hierarchy & Lifecycle**
- **System Calls:** `fork()`, `getpid()`, `getppid()`, `wait()`, `exit()`, `sleep()`.
- **Key Concepts:**
  - $2^n$ process branching factor ($n$ consecutive `fork()` calls produce $2^n$ processes).
  - Constrained process topologies: using conditional branching (`if (fork() != 0)`) to create exact counts (e.g., exactly 4 processes with 3 forks).
  - **Zombie Process:** A child terminates via `exit(0)` while parent delays calling `wait()`. The child stays in state `'Z'` in `ps` until parent collects its status.
  - **Text Processing & Basic CLI:** Parsing lines, words, characters, longest line, and Linux utilities (`mkdir`, `cd`, `grep`, `cat`).

### **Week 2: Process Replacement (`exec`), Orchestration & Orphan States**
- **System Calls:** `execl()`, `execlp()`, `execv()`, `execvp()`, `waitpid()`, `WIFEXITED()`, `WEXITSTATUS()`.
- **Key Concepts:**
  - Replacing the current process address space with a new program image.
  - Fork-Exec Controller pattern: Parent forks worker processes to execute external binaries (`cp`, custom `calc_worker`) concurrently.
  - Child exit status inspection using `waitpid(pid, &status, 0)`.
  - **Orphan Process:** Parent exits immediately; child continues running. Child is adopted by `init` or systemd user manager. Detecting orphan transition via busy-waiting on parent PID change without `sleep()`: `while(getppid() == initial_ppid);`.

### **Week 3: Inter-Process Communication via Anonymous Pipes**
- **System Calls:** `pipe(int fd[2])`, `read()`, `write()`, `close()`.
- **Key Concepts:**
  - Unidirectional stream IPC: `fd[0]` is read end, `fd[1]` is write end.
  - **Deadlock Prevention Rule:** Every process must close the ends of the pipe it does not use. If the writer's end remains open anywhere, `read()` will block forever instead of returning `0` (EOF).
  - **Bidirectional IPC:** Two separate pipes (`p2c` and `c2p`) coordinated between parent and child with custom protocols and error status handling (e.g. division by zero).

### **Week 4: Shared Memory IPC & Parallel Computing**
- **POSIX Shared Memory API (`-lrt`):**
  - `shm_open()`, `ftruncate()`, `mmap()`, `munmap()`, `shm_unlink()`.
  - Backed by `/dev/shm`.
- **System V Shared Memory API:**
  - `ftok()`, `shmget()`, `shmat()`, `shmdt()`, `shmctl(shmid, IPC_RMID, NULL)`.
  - Kernel IPC tracking via `ipcs -m`, removal via `ipcrm -m <shmid>`.
- **Parallel Computing / Worker Slicing:**
  - Allocating shared memory buffers for large data sets.
  - Partitioning rows/chunks evenly across $K$ child processes (`start = w * N / K`, `end = (w + 1) * N / K`).
  - Lock-free concurrency due to mutually exclusive memory write partitions.
  - Barrier synchronization using a `wait(NULL)` loop in the parent.

---

## 2. Priority Ranking: Concepts & Exam Weightage

| Rank | Topic | Concept Depth | Exam Probability | Likely Question Slot | Rationale |
| :---: | :--- | :---: | :---: | :---: | :--- |
| **1** | **Parallel Computing with Shared Memory** | Very High | **95%** | **Question 2 (15 Marks)** | Demands shared memory allocation (`shmget`/`mmap`), worker process slicing in a loop, barrier synchronization, verification, and deallocation. |
| **2** | **Bidirectional IPC with Two Pipes** | High | **85%** | **Question 1 (10 Marks) / Q2 (15 Marks)** | Tests descriptor lifecycle management, closing all 4 unused ends, synchronous protocol communication, and error code propagation. |
| **3** | **Process Orchestration with `fork` + `exec`** | Medium-High | **80%** | **Question 1 (10 Marks)** | Spawning standalone utilities or Linux binaries concurrently, argument passing, and exit status decoding with `waitpid()`. |
| **4** | **Unidirectional Pipe String / Data Stream** | Medium | **70%** | **Question 1 (10 Marks)** | Parent-child streaming, buffer parsing, string transformations (reverse, uppercase, vowel count). |
| **5** | **Process Topologies & Zombie / Orphan Lifecycle** | Medium | **60%** | **Question 1 (10 Marks)** | Constructing specific process trees, verifying zombie state via `wait()`, busy-loop orphan detection. |
| **6** | **Standalone POSIX vs System V Sender / Receiver** | Medium | **50%** | **Question 1 (10 Marks)** | Two distinct `.c` programs communicating via `/dev/shm` or `ftok()` key. |

---

## 3. Essential Code Templates & Complete Solutions

### 3.1 Bidirectional Pipes: Calculator Worker
> Addresses the prompt from [week3/q2](./week3/q2/README.md): Two pipes for two-way communication, graceful error handling for division by zero, all unused ends closed.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int p2c[2]; // Parent to Child pipe
    int c2p[2]; // Child to Parent pipe

    if (pipe(p2c) == -1 || pipe(c2p) == -1) {
        perror("pipe failed");
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // --- CHILD PROCESS ---
        close(p2c[1]); // Close unused write end of p2c
        close(c2p[0]); // Close unused read end of c2p

        int a, b;
        char op;
        read(p2c[0], &a, sizeof(int));
        read(p2c[0], &b, sizeof(int));
        read(p2c[0], &op, sizeof(char));
        close(p2c[0]); // Finished reading

        double result = 0.0;
        int status_code = 0; // 0: OK, 1: Division by zero, 2: Invalid op

        switch (op) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/':
                if (b == 0) status_code = 1;
                else result = (double)a / b;
                break;
            default:
                status_code = 2;
                break;
        }

        write(c2p[1], &status_code, sizeof(int));
        write(c2p[1], &result, sizeof(double));
        close(c2p[1]); // Finished writing
        exit(0);
    } else {
        // --- PARENT PROCESS ---
        close(p2c[0]); // Close unused read end of p2c
        close(c2p[1]); // Close unused write end of c2p

        int a = 12, b = 4;
        char op = '+';

        write(p2c[1], &a, sizeof(int));
        write(p2c[1], &b, sizeof(int));
        write(p2c[1], &op, sizeof(char));
        close(p2c[1]); // Signal child that data sending is complete

        int status_code;
        double result;
        read(c2p[0], &status_code, sizeof(int));
        read(c2p[0], &result, sizeof(double));
        close(c2p[0]); // Finished reading

        wait(NULL); // Reap child to avoid zombie

        if (status_code == 1) {
            printf("Error: Division by zero!\n");
        } else if (status_code == 2) {
            printf("Error: Invalid operator!\n");
        } else {
            printf("Result of %d %c %d = %.2f\n", a, op, b, result);
        }
    }
    return 0;
}
```

---

### 3.2 POSIX Shared Memory (`shm_open`, `mmap`, `-lrt`)
> Addresses the prompt from [week4/q1](./week4/q1/README.md): Modern POSIX API with `/lab_shm_basic`, 256 bytes buffer, inspectable via `/dev/shm`.

#### `posix_sender.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/lab_shm_basic"
#define SHM_SIZE 256

int main() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); return 1; }

    if (ftruncate(fd, SHM_SIZE) == -1) {
        perror("ftruncate"); return 1;
    }

    char *ptr = (char *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    printf("Enter a line of text: ");
    fgets(ptr, SHM_SIZE, stdin);

    printf("Message written. Check /dev/shm/lab_shm_basic in terminal.\n");
    printf("Press ENTER to exit sender...\n");
    getchar();

    munmap(ptr, SHM_SIZE);
    close(fd);
    return 0;
}
```

#### `posix_receiver.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/lab_shm_basic"
#define SHM_SIZE 256

int main() {
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd < 0) { perror("shm_open"); return 1; }

    char *ptr = (char *)mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    printf("Virtual Address mapped: %p\n", (void *)ptr);
    printf("Received message: %s\n", ptr);

    munmap(ptr, SHM_SIZE);
    close(fd);
    shm_unlink(SHM_NAME); // Clean up shared memory object
    printf("Shared memory unlinked successfully.\n");
    return 0;
}
```

---

### 3.3 System V Shared Memory (`ftok`, `shmget`, `shmat`)
> Addresses the prompt from [week4/q2](./week4/q2/README.md): Key generation via `ftok(".", 'B')`, attaching, detaching, inspecting via `ipcs -m`, and eviction via `shmctl IPC_RMID`.

#### `sysv_creator.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    key_t key = ftok(".", 'B');
    if (key == -1) { perror("ftok"); return 1; }

    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); return 1; }

    int *reg_no = (int *)shmat(shmid, NULL, 0);
    if (reg_no == (void *)-1) { perror("shmat"); return 1; }

    *reg_no = 241027; // Student registration number
    printf("Segment created with shmid: %d, Attached at: %p\n", shmid, (void *)reg_no);
    printf("Stored value: %d\n", *reg_no);

    shmdt(reg_no); // Detach without deleting segment
    printf("Detached. Run 'ipcs -m' in terminal to verify nattch=0.\n");
    return 0;
}
```

#### `sysv_reader.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    key_t key = ftok(".", 'B');
    if (key == -1) { perror("ftok"); return 1; }

    int shmid = shmget(key, sizeof(int), 0666);
    if (shmid < 0) { perror("shmget"); return 1; }

    int *reg_no = (int *)shmat(shmid, NULL, 0);
    if (reg_no == (void *)-1) { perror("shmat"); return 1; }

    printf("Attached to shmid: %d at address: %p\n", shmid, (void *)reg_no);
    printf("Read value: %d\n", *reg_no);

    shmdt(reg_no);
    shmctl(shmid, IPC_RMID, NULL); // Mark segment for eviction
    printf("Segment marked for deletion. Run 'ipcs -m' to verify removal.\n");
    return 0;
}
```

---

### 3.4 Parallel Matrix Multiplication with Disjoint Memory Slices
> Addresses the prompt from [week4/q3](./week4/q3/README.md): Lock-free parallel multiplication $C = A \times B$ across $K$ workers.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main() {
    int N, K;
    printf("Enter matrix dimension N: ");
    if (scanf("%d", &N) != 1) return 1;
    printf("Enter number of worker processes K (K <= %d): ", N);
    if (scanf("%d", &K) != 1) return 1;

    size_t size = N * N * sizeof(double);
    int sa = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    int sb = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
    int sc = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);

    double *A = (double *)shmat(sa, NULL, 0);
    double *B = (double *)shmat(sb, NULL, 0);
    double *C = (double *)shmat(sc, NULL, 0);

    // Initialize: A is all 1.0, B is Identity Matrix
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i * N + j] = 1.0;
            B[i * N + j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Fork K worker processes concurrently
    for (int w = 0; w < K; w++) {
        pid_t pid = fork();
        if (pid == 0) {
            int start_row = w * N / K;
            int end_row = (w + 1) * N / K;
            printf("Worker %d (PID %d): computing rows %d to %d\n", w + 1, getpid(), start_row, end_row - 1);

            for (int i = start_row; i < end_row; i++) {
                for (int j = 0; j < N; j++) {
                    C[i * N + j] = 0.0;
                    for (int k = 0; k < N; k++) {
                        C[i * N + j] += A[i * N + k] * B[k * N + j];
                    }
                }
            }
            shmdt(A); shmdt(B); shmdt(C);
            exit(0); // Worker terminates
        }
    }

    // Parent waits for all K workers (Barrier Synchronization)
    for (int w = 0; w < K; w++) {
        wait(NULL);
    }

    // Verification: Since B is Identity, C must equal A (all elements 1.0)
    int success = 1;
    for (int i = 0; i < N && success; i++) {
        for (int j = 0; j < N; j++) {
            if (C[i * N + j] != 1.0) {
                success = 0;
                break;
            }
        }
    }

    printf("All workers finished.\n");
    printf("Verification: %s\n", success ? "SUCCESS" : "FAILED");

    // Cleanup resources
    shmdt(A); shmdt(B); shmdt(C);
    shmctl(sa, IPC_RMID, NULL);
    shmctl(sb, IPC_RMID, NULL);
    shmctl(sc, IPC_RMID, NULL);

    return 0;
}
```

---

### 3.5 Process Orchestration & Exec (`fork` + `execlp` + `waitpid`)
> Pattern from [week2/q2](./week2/q2/README.md) and [week2/q3](./week2/q3/README.md):

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <src> <dst>\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Execute Linux 'cp' command
        execlp("cp", "cp", argv[1], argv[2], (char *)NULL);
        perror("execlp failed");
        exit(1);
    }

    printf("Parent continuing background work while child %d copies file...\n", pid);

    int status;
    waitpid(pid, &status, 0); // Wait specifically for this child

    if (WIFEXITED(status)) {
        printf("Child exited with code: %d\n", WEXITSTATUS(status));
    }
    return 0;
}
```

---

### 3.6 Zombie and Orphan Creation & Detection

#### A. Zombie Process Demonstration ([week1/q4](./week1/q4/README.md))
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        printf("Child (PID %d) exiting immediately to become a zombie...\n", getpid());
        exit(0);
    } else {
        printf("Parent (PID %d) sleeping for 10s without calling wait(). Check 'ps -o pid,ppid,state,cmd'\n", getpid());
        sleep(10);
        wait(NULL); // Zombie reaped here
        printf("Child collected by wait(). Zombie eradicated.\n");
    }
    return 0;
}
```

#### B. Orphan Detection without `sleep()` ([week2/q4](./week2/q4/README.md))
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    if (pid > 0) {
        printf("Parent (PID %d) terminating immediately...\n", getpid());
        exit(0);
    } else {
        pid_t initial_parent = getppid();
        // Busy wait until parent terminates and child is adopted
        while (getppid() == initial_parent);
        printf("Original Parent ID: %d\n", initial_parent);
        printf("New Adoptive Parent ID (init/systemd): %d\n", getppid());
    }
    return 0;
}
```

---

## 4. Predicted Exam Questions (10 Marks vs 15 Marks)

### **Part A: Likely 10-Mark Questions**

#### **Problem 1.1: Bidirectional Pipe String Transformation**
* **Statement:** Write a C program using two pipes (`p2c` and `c2p`) where the parent sends an arbitrary string entered by the user to the child process. The child counts vowels and consonants, reverses the string, and sends both back to the parent. The parent displays the values and calls `wait()`.
* **System Calls:** `pipe()`, `fork()`, `read()`, `write()`, `close()`, `wait()`.

#### **Problem 1.2: Multi-Process Task Runner (`fork` + `exec`)**
* **Statement:** Write a C program `task_runner` that takes a list of Linux commands as command-line arguments (e.g., `./task_runner "date" "ls -l" "whoami"`). The program must fork a child process for each command, run them concurrently using `execlp()`/`execvp()`, wait for all children to exit, and report their PID and exit codes (`WEXITSTATUS`).
* **System Calls:** `fork()`, `execvp()`, `waitpid()`, `WIFEXITED()`, `WEXITSTATUS()`.

#### **Problem 1.3: Specific Process Tree with PID/PPID Verification**
* **Statement:** Write a program using `fork()` calls to build a process tree of 4 processes: Parent $P$ forks Child $C_1$ and Child $C_2$. Child $C_1$ then forks Grandchild $G_1$. Every process must output its Role, its own PID, and its PPID.
* **System Calls:** `fork()`, `getpid()`, `getppid()`, `wait()`.

---

### **Part B: Likely 15-Mark Questions**

#### **Problem 2.1: Parallel Distributed Array Sum / Max using Shared Memory (Highest Probability)**
* **Statement:** An array of $N$ integers ($N = 1000$ or user input) is stored in shared memory. The parent allocates shared memory for the array and a results array of size $K$, then spawns $K$ worker children. Worker $w$ computes the partial sum (or local maximum) of elements in its assigned slice, and stores the value in `results[w]`. The parent waits for all $K$ workers to terminate, aggregates the final answer, compares it with sequential computation, and unlinks/destroys the shared memory.
* **System Calls:** `shmget()`, `shmat()`, `shmdt()`, `shmctl(IPC_RMID)`, `fork()`, `wait()`.

#### **Problem 2.2: Distributed Matrix Multiplication (Week 4 Lab 3)**
* **Statement:** Implement parallel matrix multiplication $C = A \times B$ for $N \times N$ matrices using $K$ worker child processes and shared memory. Workers compute non-overlapping row slices concurrently. Parent verifies $A \times I = A$ and reaps all shared resources.
* **System Calls:** `shmget()` or `shm_open()`, `mmap()`, `fork()`, `wait()`, `shmctl()`.

#### **Problem 2.3: Inter-Process Producer-Consumer with Shared Memory Flag Handshake**
* **Statement:** Write two independent programs (or parent-child) using shared memory to implement a lock-free message exchange protocol:
  - Memory structure: `struct shared_data { int status; char message[256]; };`.
  - Sender writes a message, sets `status = 1`, and waits until `status == 0`.
  - Receiver waits until `status == 1`, prints the message, sets `status = 0`.
  - Terminates after receiving `"EXIT"` and unlinks the shared memory segment.
* **System Calls:** `shm_open()`, `ftruncate()`, `mmap()`, `munmap()`, `shm_unlink()`.

---

## 5. Critical Gotchas & Compilation Cheat Sheet

### 1. Linking POSIX Real-Time Library
* **Issue:** `undefined reference to 'shm_open'` or `shm_unlink`.
* **Fix:** Add `-lrt` when compiling:
  ```bash
  gcc program.c -o program -lrt
  ```

### 2. Pipe Deadlocks (Hanging `read()`)
* If child reads until EOF, the **write end must be closed in all processes**:
  ```c
  close(p2c[1]); // In child: close write end immediately
  // ... parent writes ...
  close(p2c[1]); // In parent: close write end when writing is done
  ```
  If parent forgets `close(p2c[1])`, the child's `read()` will block forever!

### 3. Concurrent vs Sequential Worker Execution
* **Wrong (Runs Sequentially):**
  ```c
  for (int i = 0; i < K; i++) {
      if (fork() == 0) { work(); exit(0); }
      wait(NULL); // BUG: waits immediately, defeats parallel execution!
  }
  ```
* **Correct (Runs Truly in Parallel):**
  ```c
  for (int i = 0; i < K; i++) {
      if (fork() == 0) { work(); exit(0); }
  }
  for (int i = 0; i < K; i++) {
      wait(NULL); // Barrier wait after all workers are launched
  }
  ```

### 4. Cleaning Leaked System V Memory Segments
* Inspect: `ipcs -m`
* Manually remove: `ipcrm -m <shmid>`
* Inspect POSIX objects: `ls -l /dev/shm`, remove via `rm /dev/shm/<name>`
