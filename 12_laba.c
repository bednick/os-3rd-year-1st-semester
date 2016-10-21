#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SIZEPRINT 10

pthread_mutex_t lock; 
pthread_cond_t cond; 

void decrement_count(int iter);
void increment_count(int iter);

void * thread_body(void * param) {
    int i;
    pthread_mutex_lock(&lock);
    for(i = 0; i <= SIZEPRINT; ++i){
        pthread_cond_signal(&cond);
        printf("Child  %d\n", i);
        if(i != SIZEPRINT){
            pthread_cond_wait(&cond, &lock);
        }
    }
    pthread_mutex_unlock(&lock);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    int i;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_lock(&lock);
    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    for(i = 0; i <= SIZEPRINT; ++i){
        pthread_cond_signal(&cond);
        printf("Parent  %d\n", i);
        if(i != SIZEPRINT){
            pthread_cond_wait(&cond, &lock);
        }
    }
    pthread_mutex_unlock(&lock);
    pthread_join(thread, NULL);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
}