#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#define SIZEBUFPUT 80
#define PUT 1
#define GET 0
#define TIMESTEPPUT 1
#define TIMESTEPGET 10
#define SIZESTEP 10
#define MAXSIZEQUEUE 10

struct Vertex {
    char* string;
    struct Vertex* next;
};

typedef struct Queue {
    int counter;
    pthread_cond_t cond_counter;
    pthread_mutex_t mut_counter;

    struct Vertex* head;
    struct Vertex* tail;
    char is_drop;
} Queue;

struct Param {
    char get_or_put;
    int ID;
    Queue* queue;
};

void mymsginit(Queue *);
void mymsqdrop(Queue *);
void mymsgdestroy(Queue *);
int mymsgput(Queue *, char * msg);
int mymsgget(Queue *, char * buf, size_t bufsize);

int start_thread(pthread_t* pthread, struct Param* param);
void* thread_body(void* raram);
struct Param* get_param(char get_or_put, int ID, Queue* queue);

int main(int argc, char* argv) {
    int i;
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    struct Param** params = (struct Param**)malloc(sizeof(struct Param*)*4);
    pthread_t** pthreads = (pthread_t**)malloc(sizeof(pthread_t*)*4);
    mymsginit(queue);
    for(i=0; i<4; ++i){
        pthreads[i] = (pthread_t*)malloc(sizeof(pthread_t));
        if(i < 2){
            params[i] = get_param(PUT, i, queue);
        } else {
            params[i] = get_param(GET, i, queue);
        }
        start_thread(pthreads[i], params[i]);
    }

    for(i = 0; i < 4; ++i){
        pthread_join(*pthreads[i], NULL);
        free(params[i]);
        free(pthreads[i]);
    }
    mymsgdestroy(queue);
    free(params);
    free(pthreads);

}

void mymsginit(Queue* queue) {
    pthread_cond_init(&queue->cond_counter, NULL);
    pthread_mutex_init(&queue->mut_counter, NULL);
    queue->counter = 0;
    queue->head = NULL;
    queue->tail = NULL;
    queue->is_drop = 0;
}

void mymsqdrop(Queue *queue) {
    queue->is_drop = 1;
    pthread_cond_broadcast(&queue->cond_counter);
}

void mymsgdestroy(Queue *queue) {
    int i;
    struct Vertex* ver = queue->head;
    struct Vertex* next;
    for(; ver != NULL; ver = next){
        next = ver->next;
        free(ver);
    }
    queue->is_drop = 1;
    queue->head = NULL;
    queue->tail = NULL;
    pthread_cond_destroy(&queue->cond_counter);
    pthread_mutex_destroy(&queue->mut_counter);
}

int mymsgput(Queue *queue, char * msg) {
    struct Vertex* newVertex;
    int size_str;
    if(queue->is_drop){
        return 0;
    }
    if((newVertex = (struct Vertex*)malloc(sizeof(struct Vertex))) == NULL ){
        return -1;
    }
    newVertex->next = NULL;
    newVertex->string = (char*)malloc(sizeof(char)*SIZEBUFPUT);
    size_str = strlen(strncpy(newVertex->string, msg, SIZEBUFPUT));


    pthread_mutex_lock(&queue->mut_counter);
    while(1){
        if(queue->is_drop){
            pthread_mutex_unlock(&queue->mut_counter);
            return 0;
        }
        if(queue->counter < MAXSIZEQUEUE){
            break;
        }
        pthread_cond_wait(&queue->cond_counter,&queue->mut_counter);
    }

    if(queue->head == NULL) {
        queue->tail = newVertex;
        queue->head = newVertex;
    } else {
        queue->tail->next = newVertex;
        queue->tail = newVertex;
    }
    ++(queue->counter);
    pthread_mutex_unlock(&queue->mut_counter);
    pthread_cond_broadcast(&queue->cond_counter);
    return size_str;
}

int mymsgget(Queue *queue, char * buf, size_t bufsize) {
    struct Vertex* ver_get;
    int size;
    if(queue->is_drop){
        return 0;
    }

    pthread_mutex_lock(&queue->mut_counter);
    while(1){
        if(queue->is_drop){
            pthread_mutex_unlock(&queue->mut_counter);
            return 0;
        }
        if(queue->counter > 0){
            break;
        }
        pthread_cond_wait(&queue->cond_counter,&queue->mut_counter);
    }
    ver_get = queue->head;
    if(queue->head->next == NULL){
        if(queue->head->next == NULL){
            queue->head = NULL;
            queue->tail = NULL;
        }
    } else {
        queue->head = queue->head->next;
    }
    --(queue->counter);
    pthread_mutex_unlock(&queue->mut_counter);
    pthread_cond_broadcast(&queue->cond_counter);
    size =  strlen(strncpy(buf, ver_get->string, bufsize));
    free(ver_get);
    return size;
}

int start_thread(pthread_t* pthread, struct Param* param) {
    int code;
    code = pthread_create(pthread, NULL, thread_body, param);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, " creating thread: %s\n", buf);
        return 1;
    }
    return 0;
}

void* thread_body(void* param_) {
    int i;
    struct Param* param = (struct Param*)param_;
    if(param->get_or_put == PUT){
        for(i=0; i < SIZESTEP; ++i){
            sleep(TIMESTEPPUT);
            fprintf(stderr, "put i=%d\n", i);
            mymsgput(param->queue, "string111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111");
        }
    } else if(param->get_or_put == GET){
        for(i=0; i < SIZESTEP; ++i){
            size_t bufsize = 10*i;
            sleep(TIMESTEPGET);
            fprintf(stderr, "get i=%d\n", i);
            char *buf = (char*)malloc(sizeof(char) * bufsize);
            mymsgget(param->queue,buf,bufsize);
            fprintf(stderr, "%s\n", buf);
            free(buf);
        }
    } else {
        fprintf(stderr, "error param thread\n");
    }
    return NULL;
}

struct Param* get_param(char get_or_put, int ID, Queue* queue) {
    struct Param* param = (struct Param*)malloc(sizeof(struct Param));
    param->get_or_put = get_or_put;
    param->ID = ID;
    param->queue = queue;
    return param;
}
