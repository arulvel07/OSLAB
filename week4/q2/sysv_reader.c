// CS24i1027
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main() {
    // 1. Use matching ftok(".", 'B') parameters to locate key
    key_t key = ftok(".", 'B');
    if (key == -1) {
        perror("ftok failed");
        return 1;
    }

    // 2. Locate the shmid
    int shmid = shmget(key, sizeof(int), 0666);
    if (shmid < 0) {
        perror("shmget failed");
        return 1;
    }

    // 3. Attach to segment, read and display integer along with virtual address
    int *reg_no = (int *)shmat(shmid, NULL, 0);
    if (reg_no == (void *)-1) {
        perror("shmat failed");
        return 1;
    }

    printf("Attached to shmid: %d at Virtual Address: %p\n", shmid, (void *)reg_no);
    printf("Read Register Number: %d\n", *reg_no);

    // 4. Detach via shmdt()
    if (shmdt(reg_no) == -1) {
        perror("shmdt failed");
        return 1;
    }

    // 5. Mark segment for eviction via shmctl(shmid, IPC_RMID, NULL)
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failed");
        return 1;
    }

    printf("Segment successfully marked for eviction. Verify via 'ipcs -m'.\n");
    return 0;
}
