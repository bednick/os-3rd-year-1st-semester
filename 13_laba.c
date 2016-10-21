#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#define SIZE_PRINT 50000000

sem_t sem_a, sem_b;

void * thread_body(void * param) {
    int i;
    for(i = 0; i < SIZE_PRINT; ++i){
        sem_wait(&sem_b);
        fprintf(stderr, "C");
        sem_post(&sem_a);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    int i;
    sem_init(&sem_a, 0, 0);
    sem_init(&sem_b, 0, 1);

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    for(i = 0; i < SIZE_PRINT; ++i){
        sem_wait(&sem_a);
        fprintf(stderr,"P");
        sem_post(&sem_b);
    }

    pthread_join(thread, NULL);
    sem_destroy(&sem_a);
    sem_destroy(&sem_b);
    return 0;
}