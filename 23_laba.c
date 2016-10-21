#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#define SIZEBUFPUT 80
#define PUT 1
#define GET 0
#define TIMESTEPPUT 1
#define TIMESTEPGET 5
#define SIZESTEP 10
#define MAXSIZEQUEUE 10

struct Vertex {
    char* string;
    struct Vertex* next;
};

typedef struct Queue {
    sem_t counter;
    sem_t sem_head;
    sem_t sem_tail;
    sem_t sem_max;
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
//    sleep(15);
//    mymsqdrop(queue);
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
    sem_init(&queue->counter, 0, 0);
    sem_init(&queue->sem_head, 0, 1);
    sem_init(&queue->sem_tail, 0, 1);
    sem_init(&queue->sem_max, 0, MAXSIZEQUEUE);
    queue->head = NULL;
    queue->tail = NULL;
    queue->is_drop = 0;
}

void mymsqdrop(Queue *queue) {
    queue->is_drop = 1;
    sem_post(&queue->sem_max);
    sem_post(&queue->counter);
}

void mymsgdestroy(Queue *queue) {
    int i;
    struct Vertex* ver = queue->head;
    struct Vertex* next;

    for(sem_getvalue(&queue->counter, &i); i > 0; --i){
        next = ver->next;
        free(ver);
        ver = next;
    }
    queue->is_drop = -1;
    queue->head = NULL;
    queue->tail = NULL;
    sem_destroy(&queue->counter);
    sem_destroy(&queue->sem_head);
    sem_destroy(&queue->sem_tail);
    sem_destroy(&queue->sem_max);
}

int mymsgput(Queue *queue, char * msg) {
    struct Vertex* newVertex = (struct Vertex*)malloc(sizeof(struct Vertex));
    int size;
    if(queue->is_drop){
        return 0;
    }
    newVertex->next = NULL;
    newVertex->string = (char*)malloc(sizeof(char)*SIZEBUFPUT);
    size = strlen(strncpy(newVertex->string, msg, SIZEBUFPUT));
    sem_wait(&queue->sem_max);
    if(queue->is_drop){
        return 0;
    }
    sem_wait(&queue->sem_tail);
    if(queue->head == NULL) {
        queue->tail = newVertex;
        queue->head = newVertex;
        sem_post(&queue->counter);
    } else {
        queue->tail->next = newVertex;
        queue->tail = newVertex;
        sem_post(&queue->counter);
    }
    sem_post(&queue->sem_tail);
    return size;
}

int mymsgget(Queue *queue, char * buf, size_t bufsize) {
    struct Vertex* ver_get;
    int size;
    if(queue->is_drop){
        return 0;
    }
    sem_wait(&queue->counter);
    if(queue->is_drop){
        return 0;
    }
    sem_wait(&queue->sem_head);
    ver_get = queue->head;
    if(queue->head->next == NULL){
        sem_wait(&queue->sem_tail);
        if(queue->head->next == NULL){
            queue->head = NULL;
            queue->tail = NULL;
        }
        sem_post(&queue->sem_tail);
    } else {
        queue->head = queue->head->next;
    }
    sem_post(&queue->sem_head);
    sem_post(&queue->sem_max);

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
