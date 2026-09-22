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
