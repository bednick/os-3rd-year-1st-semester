#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#define SIZESTEP 10
struct Queue{
    sem_t counter;
};

struct Param {
    int size_source;
    int time_sleep;
    struct Queue* source_1;
    struct Queue* source_2;
    struct Queue* result;
    char* string_ID;
    char* string_result;
};

int queue_init(struct Queue** queue);
int queue_destroy(struct Queue* queue);
int param_init(struct Param** param, int size_source, struct Queue* source_1,  struct Queue* source_2,  struct Queue* result, char* ID, char* str_res, int time_sleep);
void param_destroy(struct Param* param);
int create_thread(pthread_t* pthread, struct Param* param);
void* thread_body(void* param);

int main(int argc, char* argv) {
    struct Queue *queue_A, *queue_B, *queue_C, *queue_AB;
    struct Param *provider_A, *provider_B, *provider_C, *provider_AB, *provider_result;
    pthread_t pthread_A, pthread_B, pthread_C, pthread_AB, pthread_result;

    if(queue_init(&queue_A) || queue_init(&queue_B) || queue_init(&queue_C) || queue_init(&queue_AB)){
        return -1;
    }
    if(param_init(&provider_A, 0, NULL, NULL, queue_A, "provider_A", "detail A", 1)){
        return -1;
    }
    if(param_init(&provider_B, 0, NULL, NULL, queue_B, "provider_B", "detail B", 2)){
        return -1;
    }

    if(param_init(&provider_C, 0, NULL, NULL, queue_C, "provider_C", "detail C", 3)){
        return -1;
    }
    if(param_init(&provider_AB, 2, queue_A, queue_B, queue_AB, "fabricator_AB", "detail AB", 0)){
        return -1;
    }
    if(param_init(&provider_result, 2, queue_C, queue_AB, NULL, "fabricator_widget", "widget", 0)){
        return -1;
    }

    if(create_thread(&pthread_A ,provider_A) || create_thread(&pthread_B, provider_B) || create_thread(&pthread_C, provider_C) || create_thread(&pthread_AB, provider_AB) || (create_thread(&pthread_result, provider_result))){
        return -1;
    }
fprintf(stderr, "finish init\n");

    pthread_join(pthread_A, NULL);
    pthread_join(pthread_B, NULL);
    pthread_join(pthread_C, NULL);
    pthread_join(pthread_AB, NULL);
    pthread_join(pthread_result, NULL);
    param_destroy(provider_A);
    param_destroy(provider_B);
    param_destroy(provider_C);
    param_destroy(provider_AB);
    param_destroy(provider_result);
    queue_destroy(queue_A);
    queue_destroy(queue_B);
    queue_destroy(queue_C);
    queue_destroy(queue_AB);

}

int queue_init(struct Queue** queue) {
    (*queue) = (struct Queue*)malloc(sizeof(struct Queue));
    if(queue == NULL){
        return -1;
    }
    return sem_init(&(*queue)->counter, 0, 0);
}

int queue_destroy(struct Queue* queue) {
    int res_destroy;
    res_destroy = sem_destroy(&queue->counter);
    free(queue);
    return res_destroy;
}

int param_init(struct Param** param, int size_source, struct Queue* source_1,  struct Queue* source_2,  struct Queue* result, char* ID, char* str_res, int time_sleep) {
    (*param) = (struct Param*)malloc(sizeof(struct Param));
    if(param == NULL){
        return -1;
    }
    (*param)->size_source = size_source;
    (*param)->source_1 = source_1;
    (*param)->source_2 = source_2;
    (*param)->result = result;
    (*param)->string_ID = ID;
    (*param)->string_result = str_res;
    (*param)->time_sleep = time_sleep;
    return 0;
}

void param_destroy(struct Param* param) {
    free(param);
}

int create_thread(pthread_t* pthread, struct Param* param) {
    int code;
    code = pthread_create(pthread, NULL, thread_body, param);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", param->string_ID, buf);
        return 1;
    }
    return 0;
}

void* thread_body(void* _param) {
    int i;
    struct Param* param = (struct Param*)(_param);
fprintf(stderr, "start		%s\n", param->string_ID);
    for(i = 0; i < SIZESTEP; ++i){
        if(param->size_source){
            if(param->size_source >= 1 && (param->source_1 != NULL)){
//                fprintf(stderr, "пытаемся взять 1 деталь %s\n", param->string_ID);
                sem_wait(&(param->source_1->counter));
//                fprintf(stderr, "взяли 1 деталь %s\n", param->string_ID);
            }
            if(param->size_source == 2 && (param->source_2 != NULL)){
//                fprintf(stderr, "пытаемся взять 2 деталь %s\n", param->string_ID);
                sem_wait(&(param->source_2->counter));
//                fprintf(stderr, "взяли 2 деталь  %s\n", param->string_ID);
            }
        }
        sleep(param->time_sleep);

        if(param->result != NULL){
            sem_post(&(param->result->counter));
//            fprintf(stderr, "+ %s\n", param->string_ID);
        }
        fprintf(stderr, "create		%s\n", param->string_result);
    }
    pthread_exit(0);
}