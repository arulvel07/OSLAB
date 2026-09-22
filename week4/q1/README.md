# Week 4 - Question 1: POSIX Shared Memory String Broadcaster

## Problem Statement
**Objective:** Master the fundamental lifecycle of POSIX shared memory (`shm_open`, `ftruncate`, `mmap`, `munmap`, `shm_unlink`) and examine its virtual filesystem backing under Linux (`/dev/shm`).

### Task Specifications:
1. Develop two standalone programs: `posix_sender.c` and `posix_receiver.c`.
2. `posix_sender.c` must:
   - Create a shared memory object named `/lab_shm_basic` with mode `0666` using `shm_open(..., O_CREAT | O_RDWR, 0666)`.
   - Set its storage capacity to exactly 256 bytes using `ftruncate()`.
   - Map the region into the calling process address space using `mmap()` with `PROT_READ | PROT_WRITE` and `MAP_SHARED`.
   - Prompt the user via `fgets()` to input a line of text, copy it into the mapped buffer, and pause using `getchar()` prior to exiting.
3. `posix_receiver.c` must:
   - Open `/lab_shm_basic` in read-only mode (`O_RDONLY`).
   - Map the segment using `PROT_READ`.
   - Print the sender's text, display the virtual memory address returned by `mmap()`, unmap the region, and invoke `shm_unlink()`.
4. Terminal Verification:
   - While `posix_sender` is paused, open a secondary terminal. Inspect the entry via `ls -la /dev/shm` and inspect the content via `cat /dev/shm/lab_shm_basic`.

![Question](./Screenshot%202026-09-22%20140211.png)

---

## 📚 POSIX API Syntax & Quick Reference

### 1. `shm_open()` — Create or open a POSIX shared memory object
```c
int shm_open(const char *name, int oflag, mode_t mode);
```
* **Header:** `<sys/mman.h>`, `<fcntl.h>`
* **Parameters:**
  * `name`: Name starting with `/` (e.g., `"/lab_shm_basic"`). Backed in Linux by `/dev/shm/`.
  * `oflag`: Bitwise-OR flags controlling open mode (`O_RDONLY`, `O_RDWR`, `O_CREAT`, etc.).
  * `mode`: File permission bits (e.g., `0666` for read/write by all). Required whenever `O_CREAT` is passed.
* **Returns:** File descriptor (`fd >= 0`) on success, `-1` on error.

### 2. `ftruncate()` — Set the size of the shared memory object
```c
int ftruncate(int fd, off_t length);
```
* **Header:** `<unistd.h>`
* **Parameters:**
  * `fd`: File descriptor returned by `shm_open()`.
  * `length`: Size in bytes (a newly created shared memory object starts with size 0, so `ftruncate()` is mandatory before mapping).
* **Returns:** `0` on success, `-1` on error.

### 3. `mmap()` — Map shared memory into process virtual address space
```c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```
* **Header:** `<sys/mman.h>`
* **Parameters:**
  * `addr`: Suggested starting address (pass `NULL` to let the kernel choose).
  * `length`: Number of bytes to map.
  * `prot`: Memory protection flags (`PROT_READ`, `PROT_WRITE`, etc.).
  * `flags`: Sharing behavior (`MAP_SHARED` for inter-process communication).
  * `fd`: File descriptor from `shm_open()`.
  * `offset`: Byte offset inside the object (typically `0`).
* **Returns:** Pointer to mapped virtual address on success, `MAP_FAILED` (`(void *)-1`) on error.

### 4. `munmap()` — Unmap memory from process address space
```c
int munmap(void *addr, size_t length);
```
* **Parameters:**
  * `addr`: Starting address returned by `mmap()`.
  * `length`: Same size passed to `mmap()`.
* **Returns:** `0` on success, `-1` on error.

### 5. `shm_unlink()` — Remove shared memory object from the system
```c
int shm_unlink(const char *name);
```
* **Parameters:**
  * `name`: The POSIX object name (e.g., `"/lab_shm_basic"`).
* **Behavior:** Removes the object from `/dev/shm`. The memory persists until all open processes unmap and close it, after which it is freed.
* **Returns:** `0` on success, `-1` on error.

---

## 🔑 Core Flags Cheat Sheet & The Mental Model

> **The Golden Rule to avoid confusing flags:**
> * `O_*` = **How to open/create** the object (`shm_open`)
> * `PROT_*` = **What access rights** the process has on memory pages (`mmap`)
> * `MAP_*` = **How changes propagate** across processes (`mmap`)

### 1. `shm_open()` Flags (`O_*`)
| Flag | Meaning | Typical Usage |
| :--- | :--- | :--- |
| `O_RDONLY` | Open for reading only | Receiver / Consumer process |
| `O_WRONLY` | Open for writing only | Writer only (rarely used alone in SHM) |
| `O_RDWR` | Open for reading and writing | Sender / Creator process |
| `O_CREAT` | Create object if it doesn't already exist | Sender / Creator (requires `0666` mode argument) |
| `O_EXCL` | Fail (`-1`) if object already exists | Ensures exclusive creation of a fresh object |
| `O_TRUNC` | Truncate existing object size to 0 | Resets existing object content |

### 2. `mmap()` Memory Protection Flags (`PROT_*`)
| Flag | Allowed Operations | Typical Usage |
| :--- | :--- | :--- |
| `PROT_READ` | Process can read mapped memory | Receiver (`mmap`) |
| `PROT_WRITE` | Process can write to mapped memory | Writer |
| `PROT_READ \| PROT_WRITE` | Process can read and write | Sender / Creator (`mmap`) |
| `PROT_NONE` | Memory pages cannot be accessed | Memory guard pages |

### 3. `mmap()` Mapping Behavior Flags (`MAP_*`)
| Flag | Behavior | In POSIX IPC? |
| :--- | :--- | :---: |
| `MAP_SHARED` | Writes are visible to all other processes mapping the same object | **MANDATORY (✅)** |
| `MAP_PRIVATE` | Copy-on-Write: Writes create private copies, never visible to others | **DO NOT USE (❌)** |
| `MAP_ANONYMOUS` | Memory not backed by any file/name (used with `fork()`, `fd = -1`) | Optional for related fork |

---

## 🔥 The Two Combinations to Know Cold for the Exam

### **1. Sender / Creator (Writing to SHM):**
```c
// 1. Open with Create + Read/Write
int fd = shm_open("/lab_shm_basic", O_CREAT | O_RDWR, 0666);

// 2. Set size (mandatory after creation)
ftruncate(fd, 256);

// 3. Map for Read + Write with MAP_SHARED
char *ptr = (char *)mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
```

### **2. Receiver / Reader (Reading from SHM & Cleanup):**
```c
// 1. Open existing object for Read-Only
int fd = shm_open("/lab_shm_basic", O_RDONLY, 0666);

// 2. Map for Read-Only with MAP_SHARED
char *ptr = (char *)mmap(NULL, 256, PROT_READ, MAP_SHARED, fd, 0);

// ... read data ...

// 3. Unmap, close and unlink
munmap(ptr, 256);
close(fd);
shm_unlink("/lab_shm_basic"); // Removes object from /dev/shm
```

---

## Solution Code

### 1. `posix_sender.c`
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
    if (fd < 0) {
        perror("shm_open failed");
        return 1;
    }

    if (ftruncate(fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    char *ptr = (char *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    printf("Enter a line of text: ");
    if (fgets(ptr, SHM_SIZE, stdin) == NULL) {
        perror("fgets failed");
    }

    printf("Message stored into shared memory successfully.\n");
    printf("Inspect now via 'ls -la /dev/shm' and 'cat /dev/shm/lab_shm_basic'\n");
    printf("Press ENTER to exit sender...\n");
    getchar();

    munmap(ptr, SHM_SIZE);
    close(fd);
    return 0;
}
```

### 2. `posix_receiver.c`
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
    if (fd < 0) {
        perror("shm_open failed");
        return 1;
    }

    char *ptr = (char *)mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    printf("Virtual Address mapped: %p\n", (void *)ptr);
    printf("Sender's text: %s\n", ptr);

    munmap(ptr, SHM_SIZE);
    close(fd);

    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink failed");
        return 1;
    }

    printf("Shared memory object '%s' successfully unlinked.\n", SHM_NAME);
    return 0;
}
```

---

## How to Compile & Verify
> **Important:** Always pass `-lrt` to link the POSIX Realtime library!

```bash
# Terminal 1: Compile and run sender
gcc posix_sender.c -o posix_sender -lrt
./posix_sender

# Terminal 2: Inspect Linux virtual filesystem backing
ls -la /dev/shm
cat /dev/shm/lab_shm_basic

# Run receiver to read and unlink
gcc posix_receiver.c -o posix_receiver -lrt
./posix_receiver

# Verify removal
ls -la /dev/shm
```
