# Week 4 - Question 2: System V Segment Registry & Kernel Inspection

## Problem Statement
**Objective:** Understand System V IPC key generation via `ftok()`, segment allocation via `shmget()`, virtual address attachment via `shmat()`, and kernel IPC table tracking via `ipcs` and `ipcrm`.

### Task Specifications:
1. Develop two standalone programs: `sysv_creator.c` and `sysv_reader.c`.
2. `sysv_creator.c` must:
   - Generate an IPC key using `ftok(".", 'B')`.
   - Allocate a shared memory segment of `sizeof(int)` bytes with flags `IPC_CREAT | 0666` via `shmget()`.
   - Attach the segment using `shmat()` and store an integer (e.g., student register number).
   - Output the generated `shmid` and the virtual address where the segment was attached.
   - Detach using `shmdt()`. DO NOT invoke `shmctl()` deletion. Terminate execution.
3. Shell Inspection Checkpoint:
   - Execute `ipcs -m` in your terminal. Locate the segment by its `shmid`. Record the value of the `nattch` column when neither process is active.
4. `sysv_reader.c` must:
   - Use the matching `ftok(".", 'B')` parameters to locate the `shmid`.
   - Attach to the segment, read and display the integer along with its virtual address.
   - Detach via `shmdt()` and mark the segment for eviction via `shmctl(shmid, IPC_RMID, NULL)`.
5. Verify removal by running `ipcs -m` again.

![Question](./Screenshot%202026-09-22%20140252.png)

---

## Solution Code

### 1. `sysv_creator.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main() {
    key_t key = ftok(".", 'B');
    if (key == -1) {
        perror("ftok failed");
        return 1;
    }

    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    int *reg_no = (int *)shmat(shmid, NULL, 0);
    if (reg_no == (void *)-1) {
        perror("shmat failed");
        return 1;
    }

    *reg_no = 241027; // Student Register Number

    printf("Generated Key: 0x%x\n", key);
    printf("Shared Memory ID (shmid): %d\n", shmid);
    printf("Attached at Virtual Address: %p\n", (void *)reg_no);
    printf("Stored Register Number: %d\n", *reg_no);

    if (shmdt(reg_no) == -1) {
        perror("shmdt failed");
        return 1;
    }

    printf("Detached segment. Run 'ipcs -m' in terminal to record nattch=0.\n");
    return 0;
}
```

### 2. `sysv_reader.c`
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main() {
    key_t key = ftok(".", 'B');
    if (key == -1) {
        perror("ftok failed");
        return 1;
    }

    int shmid = shmget(key, sizeof(int), 0666);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    int *reg_no = (int *)shmat(shmid, NULL, 0);
    if (reg_no == (void *)-1) {
        perror("shmat failed");
        return 1;
    }

    printf("Attached to shmid: %d at Virtual Address: %p\n", shmid, (void *)reg_no);
    printf("Read Register Number: %d\n", *reg_no);

    if (shmdt(reg_no) == -1) {
        perror("shmdt failed");
        return 1;
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failed");
        return 1;
    }

    printf("Segment successfully marked for eviction. Verify via 'ipcs -m'.\n");
    return 0;
}
```

---

## How to Compile & Verify
```bash
# 1. Compile both files
gcc sysv_creator.c -o sysv_creator
gcc sysv_reader.c -o sysv_reader

# 2. Run creator
./sysv_creator

# 3. Check kernel IPC registry (Observe nattch = 0)
ipcs -m

# 4. Run reader (Reads and evicts segment)
./sysv_reader

# 5. Verify segment eviction from kernel table
ipcs -m
```
