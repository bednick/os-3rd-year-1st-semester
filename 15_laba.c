#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>

#define SIZE_PRINT 50000000

int main(int argc, char *argv[]) {
    char* name_sem_a = "/sem_a";
    char* name_sem_b = "/sem_b";
    int i;
    pthread_t thread;
    sem_t *sem_a;
    sem_t *sem_b;
    pid_t pid;

    sem_a = sem_open(name_sem_a, O_CREAT | O_EXCL, O_RDWR, 1);
    if(sem_a == SEM_FAILED){
        perror(name_sem_a);
        exit(-1);
    }
    sem_b = sem_open(name_sem_b, O_CREAT | O_EXCL, O_RDWR, 0);
    if(sem_b == SEM_FAILED){
        perror(name_sem_b);
        sem_close(sem_a);
        sem_unlink(name_sem_a);
        exit(-1);
    }

    switch (pid = fork()) {
        case -1:
            perror("fork");
            break;
        case 0:
            for(i = 0; i < SIZE_PRINT; ++i){
                sem_wait(sem_b);
                fprintf(stderr,"C");
                sem_post(sem_a);
            }
            break;
        default:

        for(i = 0; i < SIZE_PRINT; ++i){
            sem_wait(sem_a);
            fprintf(stderr,"P");
            sem_post(sem_b);
        }
    }
    sem_close(sem_a);
    sem_close(sem_b);
    sem_unlink(name_sem_a);
    sem_unlink(name_sem_b);
    return 0;
}