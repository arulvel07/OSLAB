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

## 📚 System V Shared Memory API Syntax & Quick Reference

### 1. `ftok()` — Generate a System V IPC Key
```c
key_t ftok(const char *pathname, int proj_id);
```
* **Header:** `<sys/ipc.h>`
* **Parameters:**
  * `pathname`: Path to an existing, accessible file or directory (e.g., `"."` for current directory).
  * `proj_id`: A single character project identifier (e.g., `'B'`). Only the lower 8 bits are used.
* **Returns:** A unique `key_t` integer key on success, `-1` on error.
* **Exam Note:** Both the creator and reader must pass the **exact same pathname and proj_id** to generate the identical key!

---

### 2. `shmget()` — Allocate or locate a shared memory segment
```c
int shmget(key_t key, size_t size, int shmflg);
```
* **Headers:** `<sys/ipc.h>`, `<sys/shm.h>`
* **Parameters:**
  * `key`: Key returned by `ftok()`, or `IPC_PRIVATE` (used for parent-child processes).
  * `size`: Size of the shared memory region in bytes (e.g., `sizeof(int)` or `N * sizeof(double)`).
  * `shmflg`: Bitwise-OR flags controlling creation and permissions (e.g., `IPC_CREAT | 0666`).
* **Returns:** Shared memory identifier (`shmid >= 0`) on success, `-1` on error.

---

### 3. `shmat()` — Attach shared memory segment to process virtual address space
```c
void *shmat(int shmid, const void *shmaddr, int shmflg);
```
* **Header:** `<sys/shm.h>`
* **Parameters:**
  * `shmid`: The ID returned by `shmget()`.
  * `shmaddr`: Target virtual address. **Always pass `NULL` (or `0`)** so the kernel automatically chooses an aligned, free virtual address.
  * `shmflg`: Access flags:
    * `0`: Read and write access.
    * `SHM_RDONLY`: Attach as read-only.
* **Returns:** Pointer to the attached shared memory on success, or `(void *)-1` on failure (check with `if (ptr == (void *)-1)`).

---

### 4. `shmdt()` — Detach shared memory from process address space
```c
int shmdt(const void *shmaddr);
```
* **Header:** `<sys/shm.h>`
* **Parameters:**
  * `shmaddr`: The pointer previously returned by `shmat()`.
* **Behavior:** Decrements the kernel attachment counter (`nattch`). The segment itself is **not destroyed**.
* **Returns:** `0` on success, `-1` on error.

---

### 5. `shmctl()` — Control, inspect, or destroy a shared memory segment
```c
int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```
* **Header:** `<sys/shm.h>`
* **Parameters:**
  * `shmid`: Segment identifier from `shmget()`.
  * `cmd`: Operation command.
    * **`IPC_RMID` (Most Important):** Marks the segment for removal. The segment is destroyed once all attached processes detach.
    * `IPC_STAT`: Copies kernel metadata (`shmid_ds`) into `buf`.
  * `buf`: Pointer to `struct shmid_ds` (pass `NULL` when using `IPC_RMID`).
* **Returns:** `0` on success, `-1` on error.

---

## 🔑 System V Flags & Scope of Exam Cheat Sheet

### 1. `shmget()` Creation & Access Flags
| Flag | Meaning | Typical Usage |
| :--- | :--- | :--- |
| `IPC_CREAT` | Create segment if it does not already exist | Creator program (`IPC_CREAT \| 0666`) |
| `IPC_EXCL` | Fail (`-1`) if segment with this key already exists | Used with `IPC_CREAT` to guarantee a brand-new segment |
| `0666` | Permission bits: Read & write for Owner, Group, Others | Required alongside `IPC_CREAT` |
| `0` (or `0666`) | Open existing segment without creation flags | Reader / Consumer program |
| `IPC_PRIVATE` | Kernel generates a private key guaranteed unique | Used in `fork()` parent-child programs (no `ftok()` needed!) |

### 2. `shmat()` Attachment Flags
| Flag | Meaning | Typical Usage |
| :--- | :--- | :--- |
| `0` | Attach with Read & Write permissions | Creator, Sender, Worker processes |
| `SHM_RDONLY` | Attach in Read-Only mode | Strict consumer / reader process |

### 3. `shmctl()` Command Flags
| Flag | Meaning | Typical Usage |
| :--- | :--- | :--- |
| `IPC_RMID` | Mark segment for destruction / eviction from kernel table | Always called at the end to avoid kernel memory leaks |
| `IPC_STAT` | Query segment metadata (owner, permissions, `nattch`) | Diagnostic inspection |

---

## 🛠️ Linux Kernel Diagnostics & `ipcs` / `ipcrm`

System V IPC resources are managed directly by the **Linux kernel table** (they persist even after your program finishes execution until explicitly deleted or rebooted).

### 1. Inspecting Active Segments (`ipcs -m`)
In terminal, run:
```bash
ipcs -m
```
Key columns to know for exam / viva:
* **`key`:** Hexadecimal key from `ftok()` (e.g., `0x4201abcd`).
* **`shmid`:** Integer segment identifier assigned by kernel.
* **`owner`:** Linux username who created the segment.
* **`bytes`:** Allocated capacity in bytes.
* **`nattch` (Number of Attached Processes):**
  * When `sysv_creator` is running and attached: `nattch = 1`.
  * After `sysv_creator` calls `shmdt()` and exits: `nattch = 0`.
  * When both reader and creator attach concurrently: `nattch = 2`.

### 2. Cleaning Leaked Segments (`ipcrm -m`)
If your program crashes before reaching `shmctl(shmid, IPC_RMID, NULL)`, the segment remains in the kernel. Delete it manually using:
```bash
ipcrm -m <shmid>
```

---

## ⚡ The System V Lifecycle Flow (Mental Model)

```
[ftok(".", 'B')] 
       ↓ key_t
[shmget(key, size, IPC_CREAT | 0666)]
       ↓ shmid
[shmat(shmid, NULL, 0)] 
       ↓ void* pointer (cast to int*, char*, etc.)
[Read / Write Shared Data]
       ↓
[shmdt(ptr)] (detach pointer; nattch drops to 0)
       ↓
[shmctl(shmid, IPC_RMID, NULL)] (evict from kernel table)
```

> **Comparison with POSIX (Exam Distinction):**
> * **System V:** Uses integer keys (`key_t`), integer IDs (`shmid`), inspected via `ipcs -m`. Does **not** require `-lrt`.
> * **POSIX:** Uses path strings (`/name`), file descriptors (`fd`), inspected via `ls -la /dev/shm`. Requires **`-lrt`** at compile time.

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
    // 1. Generate IPC key using ftok(".", 'B')
    key_t key = ftok(".", 'B');
    if (key == -1) {
        perror("ftok failed");
        return 1;
    }

    // 2. Allocate shared memory segment of sizeof(int) bytes
    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    // 3. Attach segment to virtual memory
    int *reg_no = (int *)shmat(shmid, NULL, 0);
    if (reg_no == (void *)-1) {
        perror("shmat failed");
        return 1;
    }

    // Store data (student register number)
    *reg_no = 241027;

    printf("Generated Key: 0x%x\n", key);
    printf("Shared Memory ID (shmid): %d\n", shmid);
    printf("Attached at Virtual Address: %p\n", (void *)reg_no);
    printf("Stored Register Number: %d\n", *reg_no);

    // 4. Detach without deleting segment (so reader can access it)
    if (shmdt(reg_no) == -1) {
        perror("shmdt failed");
        return 1;
    }

    printf("Detached segment. Run 'ipcs -m' in terminal to verify nattch=0.\n");
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
    // 1. Locate the exact same key using identical ftok(".", 'B') parameters
    key_t key = ftok(".", 'B');
    if (key == -1) {
        perror("ftok failed");
        return 1;
    }

    // 2. Locate existing shmid (no IPC_CREAT needed)
    int shmid = shmget(key, sizeof(int), 0666);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    // 3. Attach and read data
    int *reg_no = (int *)shmat(shmid, NULL, 0);
    if (reg_no == (void *)-1) {
        perror("shmat failed");
        return 1;
    }

    printf("Attached to shmid: %d at Virtual Address: %p\n", shmid, (void *)reg_no);
    printf("Read Register Number: %d\n", *reg_no);

    // 4. Detach segment
    if (shmdt(reg_no) == -1) {
        perror("shmdt failed");
        return 1;
    }

    // 5. Mark segment for eviction from kernel
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
# 1. Compile both standalone programs (no -lrt needed for System V)
gcc sysv_creator.c -o sysv_creator
gcc sysv_reader.c -o sysv_reader

# 2. Run creator
./sysv_creator

# 3. Check kernel IPC registry in terminal (observe segment exists and nattch = 0)
ipcs -m

# 4. Run reader (reads integer and evicts segment)
./sysv_reader

# 5. Verify segment eviction from kernel table
ipcs -m
```
