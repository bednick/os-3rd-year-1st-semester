#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void * thread_body(void * param) {
    int i;
    for(i = 0; i < 1000; ++i){
        printf("Child %d\n",i);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    int i;

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    for(i = 0; i < 1000; ++i){
        printf("Parent %d\n",i);
    }

    pthread_join(thread, NULL);
    return 0;
}