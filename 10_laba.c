#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

struct Locks{
    pthread_mutex_t* lock;
    int number_parent; //следующий
    int number_children;
} locks;

void * thread_body(void * param) {
    int i;
    locks.number_children = 2;
    pthread_mutex_lock(&(locks.lock[1]));
//ждать, пока родитель не захватит 2!!!
    while(!pthread_mutex_trylock(&(locks.lock[2]))){
        pthread_mutex_unlock(&(locks.lock[2]));
        sleep(3);
    }
//////////////////////
    for(i = 0; i < 10; ++i){
        pthread_mutex_lock(&(locks.lock[locks.number_children]));
        printf("Child %d\n",i);
        pthread_mutex_unlock (&(locks.lock[(locks.number_children + 2) % 3]));
        locks.number_children = (locks.number_children + 1) % 3;
    }
    return NULL;
}


int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    int i;

    locks.lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*3);
    pthread_mutex_init(&(locks.lock[0]), NULL);
    pthread_mutex_init(&(locks.lock[1]), NULL);
    pthread_mutex_init(&(locks.lock[2]), NULL);

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    locks.number_parent = 0;
    pthread_mutex_lock(&(locks.lock[2]));
//Проверить, что children захватил 1
    while(!pthread_mutex_trylock(&(locks.lock[1]))){
        pthread_mutex_unlock(&(locks.lock[1]));
        sleep(1);
    }
/////
    for(i = 0; i < 10; ++i){
        pthread_mutex_lock(&(locks.lock[locks.number_parent]));
        printf("Parent %d\n",i);
        pthread_mutex_unlock (&(locks.lock[(locks.number_parent + 2) % 3]));
        locks.number_parent = (locks.number_parent + 1) % 3;
    }

    pthread_join(thread, NULL);

    pthread_mutex_destroy(&(locks.lock[0]));
    pthread_mutex_destroy(&(locks.lock[1]));
    pthread_mutex_destroy(&(locks.lock[2]));
    return 0;
}