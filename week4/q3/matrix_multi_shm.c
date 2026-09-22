#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

int main(){
    int N, K;
    printf("Enter value of N:- ");
    scanf("%d", &N);
    
    printf("Enter no of workers(K<=%d):- ", N);
    scanf("%d", &K);
    
    int s = N * N * sizeof(double);
    int sa = shmget(IPC_PRIVATE, s, 0666 | IPC_CREAT);
    int sb = shmget(IPC_PRIVATE, s, 0666 | IPC_CREAT);
    int sc = shmget(IPC_PRIVATE, s, 0666 | IPC_CREAT);
    
    double *A = shmat(sa, 0, 0);
    double *B = shmat(sb, 0, 0);
    double *C = shmat(sc, 0, 0);

    for (int i=0; i<N; i++)
        for (int j=0; j<N; j++){
            A[i*N+j] = 1.0;
            B[i*N+j] = (i==j)?1.0:0.0;
        }
        
        for (int w=0; w<K; w++){
        if (fork()==0){
            int start = w*N/K;
            int end = (w+1)*N/K;

            printf("Worker %d: rows %d to %d\n", w+1, start, end-1);

            for (int i=start; i<end; i++)
                for (int j=0; j<N; j++){
                    C[i*N+j] = 0;
                    for (int k=0; k<N; k++)
                        C[i*N+j] += A[i*N+k] * B[k*N+j];
                }
            return 0;
        }
    }
    
    for (int w=0; w<K; w++) wait(NULL);

    int ok= 1;
    for (int i=0; i<N; i++)
        for (int j=0; j<N; j++)
            if (C[i*N+j]!= 1.0) ok = 0;

    printf("All workers completed the work\n");
    printf("Verification: %s\n", ok ? "SUCCESS" : "FAILED");

    shmdt(A); shmdt(B); shmdt(C);
    shmctl(sa, IPC_RMID, 0);
    shmctl(sb, IPC_RMID, 0);
    shmctl(sc, IPC_RMID, 0);
    return 0;
}