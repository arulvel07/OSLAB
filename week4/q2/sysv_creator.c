// CS24i1027
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main() {
    // 1. Generate an IPC key using ftok(".", 'B')
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

    // 3. Attach the segment using shmat() and store integer (e.g., student register number)
    int *reg_no = (int *)shmat(shmid, NULL, 0);
    if (reg_no == (void *)-1) {
        perror("shmat failed");
        return 1;
    }

    *reg_no = 241027; // Student Register Number

    // 4. Output generated shmid and virtual address where segment was attached
    printf("Generated Key: 0x%x\n", key);
    printf("Shared Memory ID (shmid): %d\n", shmid);
    printf("Attached at Virtual Address: %p\n", (void *)reg_no);
    printf("Stored Register Number: %d\n", *reg_no);

    // 5. Detach using shmdt(). DO NOT invoke shmctl() deletion here.
    if (shmdt(reg_no) == -1) {
        perror("shmdt failed");
        return 1;
    }

    printf("Detached segment. Run 'ipcs -m' in terminal to record nattch=0.\n");
    return 0;
}
