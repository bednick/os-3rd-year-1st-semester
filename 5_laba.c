#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>



static void thread_exit(void* arg) {
    fprintf(stderr, "exit\n\n");

}


void * thread_body(void * param) {
    pthread_cleanup_push(thread_exit, NULL);
//    pthread_cleanup_pop(0);
    while(1){
        fprintf(stderr,"Child");
    }
    pthread_cleanup_pop(0);
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
    sleep(1);

    pthread_cancel(thread);
    
    //fprintf(stderr,"exit\n");
    pthread_exit(&thread);
//    return 0;
}