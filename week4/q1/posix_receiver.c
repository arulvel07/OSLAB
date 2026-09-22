// CS24i1027
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/lab_shm_basic"
#define SHM_SIZE 256

int main() {
    // 1. Open '/lab_shm_basic' in read-only mode
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd < 0) {
        perror("shm_open failed");
        return 1;
    }

    // 2. Map segment using PROT_READ
    char *ptr = (char *)mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // 3. Print the sender's text and display virtual memory address returned by mmap()
    printf("Virtual Address mapped: %p\n", (void *)ptr);
    printf("Sender's text: %s\n", ptr);

    // 4. Unmap region and invoke shm_unlink()
    munmap(ptr, SHM_SIZE);
    close(fd);

    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink failed");
        return 1;
    }

    printf("Shared memory object '%s' successfully unlinked.\n", SHM_NAME);
    return 0;
}
