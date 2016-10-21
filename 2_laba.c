#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void * thread_body(void * param) {
    int i;
    for(i = 0; i < 10; ++i){
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
    code = pthread_join(thread, NULL);
    for(i = 0; i < 10; ++i){
        printf("Parent %d\n",i);
    }

    return 0;
    //pthread_exit(&thread);

}