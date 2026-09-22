// CS24i1027
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/lab_shm_basic"
#define SHM_SIZE 256

int main() {
    // 1. Create a shared memory object named '/lab_shm_basic' with mode 0666
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("shm_open failed");
        return 1;
    }

    // 2. Set its storage capacity to exactly 256 bytes
    if (ftruncate(fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return 1;
    }

    // 3. Map region into calling process address space
    char *ptr = (char *)mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // 4. Prompt user via fgets() to input a line of text
    printf("Enter a line of text: ");
    if (fgets(ptr, SHM_SIZE, stdin) == NULL) {
        perror("fgets failed");
    }

    printf("Message stored into shared memory successfully.\n");
    printf("Inspect now via 'ls -la /dev/shm' and 'cat /dev/shm/lab_shm_basic'\n");
    printf("Press ENTER to exit sender...\n");
    getchar();

    // Cleanup local mapping
    munmap(ptr, SHM_SIZE);
    close(fd);
    return 0;
}
