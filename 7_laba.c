#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define num_steps 200000000

struct Param{
    int size_threads;
    int number;
};

void * thread_body(void * param) {
    double* pi = (double*)malloc(sizeof(double));
    int i;
    for( i = (*((struct Param *)(param))).number; i < num_steps; i +=  (*((struct Param *)(param))).size_threads ){
        *pi += 1.0/(i*4.0 + 1.0);
        *pi -= 1.0/(i*4.0 + 3.0);
    }
    fprintf(stdout, "thread %d = %f\n",  (*((struct Param *)(param))).number, *pi);
    pthread_exit (pi);
}

int main(int argc, char *argv[]) {
    pthread_t* threads;
    struct Param * params;
    int code;
    int i, j;
    int size_threads;
    double *res, allRes = 0;

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
    for(i = 0; i < size_threads; ++i){
        pthread_join((threads[i]),(void**) &res);
        allRes += (*((double *)(res)));
        free(res);
    }
    free(threads);
    free(params);
    fprintf(stdout, " pi = %.13g\n", allRes * 4);
    return 0;
}