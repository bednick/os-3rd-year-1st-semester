#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

struct Param{
    int size_threads;
    int number;
};

//pthread_t* threads;
//struct Param * params;
//int size_threads = 0;
int flag = 0;
int maxiter = 0;
pthread_mutex_t maxiterlock;

void processing_SIGINT(int var){
    flag = 1;
}

void * thread_body(void * param) {
    double* pi = (double*)malloc(sizeof(double));
    int i, j = 0;
    for( i = (*((struct Param *)(param))).number; 1 ; i +=  (*((struct Param *)(param))).size_threads ){
        *pi += 1.0/(i*4.0 + 1.0);
        *pi -= 1.0/(i*4.0 + 3.0);
        j += 1;
        pthread_mutex_lock(&maxiterlock);
        if(maxiter < j){
            maxiter = j;
        }
        if(flag){
            if(maxiter <= j){
                fprintf(stderr, "thread=%d  iter=%d \n", (*((struct Param *)(param))).number, j);
                pthread_mutex_unlock(&maxiterlock);
                pthread_exit(pi);
            }
        }
        pthread_mutex_unlock(&maxiterlock);
    }
//    pthread_exit(pi);

}

int main(int argc, char *argv[]) {
    int code;
    int i, j;
    int size_threads = 0;
    double *res, allRes = 0;
    struct Param * params;
    pthread_t* threads;

    if(argc != 2){
        fprintf(stderr, "size argv != 1\n");
        return -1;
    }
    size_threads = atoi(argv[1]);
    if(size_threads <= 0){
        fprintf(stderr, "argv[1] <= 0\n");
        return -1;
    }
    threads = (pthread_t*)malloc(sizeof(pthread_t) * size_threads);
    if(threads == NULL){
        fprintf(stderr, "memory error\n");
        return -2;
    }
    params = (struct Param*)malloc(sizeof(struct Param) * size_threads);
    if(params == NULL){
        fprintf(stderr, "memory error\n");
        free(threads);
        return -2;
    }
    pthread_mutex_init (&maxiterlock, NULL);
    for(i = 0; i < size_threads; ++i){
        params[i].size_threads = size_threads;
        params[i].number = i;
        code = pthread_create(&(threads[i]), NULL, thread_body, &params[i]);
        if (code!=0) {
            char buf[256];
            strerror_r(code, buf, sizeof buf);
            fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
            free(threads);
            free(params);
            exit(1);
        }
    }
    signal(SIGINT, processing_SIGINT);

    for(i = 0; i < size_threads; ++i){
        code = pthread_join(threads[i], (void**) &res);
        if(code != 0){
            char buf[256];
            strerror_r(code, buf, sizeof buf);
            fprintf(stderr, "join thread error: %s\n", buf);
        }

        allRes += (*((double *)(res)));
        free(res);
    }
    free(threads);
    free(params);

    fprintf(stdout, " pi = %.13g\n", allRes * 4);

    pthread_mutex_destroy(&maxiterlock);
    return 0;
}