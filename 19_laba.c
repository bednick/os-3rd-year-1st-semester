#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SIZEBUF 30

#define TIMESLEEP 5
#define SIZEPTHREADS 1

struct Vertex {
    char* string;
    struct Vertex* next;
};

struct List {
    struct Vertex* head;
    struct Vertex* tail;
    unsigned int size;
    pthread_rwlock_t lock;
};

struct List_and_Pthreads {
    struct List *list;
    int size_pthread;
    pthread_t* pthreads;
};

void* thread_body(void* param);
void sorting_list(struct List* list);
void add_vertex_in_list(struct Vertex* vertex, struct List* lis);
void print_list(struct List* list);
void destroy_list(struct List* list);
int compare(char* str1, char* str2);
void swap_neighbours(struct List* list, struct Vertex* ver);
struct Vertex* init_Vertex(char* string);
struct List* init_List();
struct List_and_Pthreads* init_List_and_Pthreads(struct List* list, unsigned size_pthread);
void destroy_List_and_Pthreads(struct List_and_Pthreads* LaP);
int foo(int return_value, char* mail);


int main (int argc, char *argv[]) {
    struct List* list;
    struct List_and_Pthreads* LaP;
    int code;
    int i;

    list = init_List();
    LaP = init_List_and_Pthreads(list, SIZEPTHREADS);

    while(1){
        char* buf = (char*)malloc(sizeof(char)*SIZEBUF);
        if(fgets(buf, SIZEBUF, stdin) == NULL){
            sleep(10);
            break;
        }
        if(buf[0] == '\n'){
            print_list(list);
        } else if(buf[0] == '`'){
            break;
        } else {
            struct Vertex* ver = init_Vertex(buf);
            add_vertex_in_list(ver, list);
        }
    }
    destroy_List_and_Pthreads(LaP);
    destroy_list(list);
    return 0;
}


void * thread_body(void *param) {
    while(1){
        sleep(TIMESLEEP);
fprintf(stderr, "				start sorting\n");
        sorting_list((struct List*)param);

fprintf(stderr, "				finish sorting\n");
    }
    return NULL;
}

void sorting_list (struct List* list) {
    int i, j, size, is_swap, steps;
    struct Vertex* begin, *end, *ver1, *ver2;
    pthread_rwlock_rdlock(&(list->lock));
    size = list->size;
    pthread_rwlock_unlock(&(list->lock));

    if(size < 3){
        return;
    }
    pthread_rwlock_wrlock(&(list->lock));
    for(i = 0; i < size - 1; ++i){
        steps = 0;
        begin = NULL;
        ver1 = list->head;
        ver2 = list->head->next;
        for(j = 0; j < size - i - 1; ++j){
            is_swap = 0;
            if(compare(ver1->string, ver2->string) > 0){
                swap_neighbours(list, begin);
                is_swap = 1;
                ++steps;
            }
            if(is_swap){
                begin = ver2;
                ver2 = ver1->next;
            } else{
                begin = ver1;
                ver1 = ver2;
                ver2 = ver2->next;
            }
        }
        if(!steps){
            break;
        }
    }
    pthread_rwlock_unlock(&(list->lock));
}

void add_vertex_in_list (struct Vertex *vertex, struct List* list) {
    int size;
    pthread_rwlock_wrlock(&(list->lock));
    size = list->size;

    if(!size){
        list->head = vertex;
        list->tail = vertex;
        list->size = 1;
    } else {
        struct Vertex* var;
        var = list->tail;
        list->tail->next = vertex;
        list->tail = vertex;
        ++(list->size);
    }
    pthread_rwlock_unlock(&(list->lock));
}

void print_list(struct List* list){
    struct Vertex* var1, * var2;

    pthread_rwlock_rdlock(&(list->lock));
    fprintf(stderr, "\n************PRINT_LIST***********\n");
    if(!list->size){
        fprintf(stderr, "*********************************************\n");
        pthread_rwlock_unlock(&(list->lock));
        return;
    }
    if(list->size == 1){

        fprintf(stderr, "%s\n", list->head->string);

        fprintf(stderr, "*********************************************\n");
        pthread_rwlock_unlock(&(list->lock));
        return;
    }
    var1 = list->head;
    while(var1 != NULL){

        var2 = var1->next;
        fprintf(stderr, "%s\n", var1->string);
        var1 = var2;
    }
    fprintf(stderr, "*********************************************\n");
    pthread_rwlock_unlock(&(list->lock));
}

void destroy_list(struct List* list) {
    if(!list->size){
        pthread_rwlock_destroy(&(list->lock));
        free(list);
        return;
    }
    if(list->size == 1){
        free(list->head->string);
        free(list->head);
        pthread_rwlock_destroy(&(list->lock));
        free(list);
        return;
    }
    struct Vertex* var1 = list->head;
    struct Vertex* var2 = list->head->next;
    while(var2 != NULL){
        free(var1->string);
        free(var1);
        var1 = var2;
        var2 = var2->next;
    }

    free(var1);

    pthread_rwlock_destroy(&list->lock);
    free(list);
}

int compare(char* str1, char* str2) {
    int i;
    if(strlen(str1) <= strlen(str2)){
        for(i = 0; i < strlen(str1); ++i){
            if(str1[i] < str2[i]){
                return -1;
            } else if(str1[i] > str2[i]){
                return 1;
            }
        }
        if(strlen(str1) == strlen(str2)){
            return 0;
        } else {
            return -1;
        }
    } else {
        for(i = 0; i < strlen(str2); ++i){
            if(str1[i] < str2[i]){
                return -1;
            } else if(str1[i] > str2[i]){
                return 1;
            }
        }
        return 1;
    }
}

void swap_neighbours(struct List* list, struct Vertex* ver) {
    int i;
    struct Vertex*ver1, *ver2, *ver3;

    if(ver == NULL){
        ver2 = list->head->next;
        ver3 = list->head->next->next;
        list->head->next->next = list->head;
        list->head->next = ver3;
        list->head = ver2;
        if(list->size == 2){
            list->tail = list->head->next;
        }
        return;
    }
    ver1 = ver->next;
    ver2 = ver->next->next;
    if(ver2 == NULL){
        return;
    }
    ver3 = ver->next->next->next;
    ver2->next = ver->next;
    ver1->next = ver3;
    ver->next = ver2;
    if(ver1->next == NULL){
        list->tail = ver1;
    }
}

struct Vertex* init_Vertex(char* string) {
    struct Vertex* ver = (struct Vertex*)malloc(sizeof(struct Vertex));
    if(ver == NULL){
        fprintf(stderr, "memory Vertex error\n");
        return NULL;
    }
    ver->string = string;
    ver->next = NULL;
    return ver;
}

struct List* init_List() {
    struct List* list = (struct List*)malloc(sizeof(struct List));
    if(list == NULL){
        fprintf(stderr, "memory List error\n");
        return NULL;
    }
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    pthread_rwlock_init (&list->lock, NULL);

    return list;
}

struct List_and_Pthreads* init_List_and_Pthreads(struct List* list, unsigned size_pthread) {
    int code, i, j;
    struct List_and_Pthreads* LaP = (struct List_and_Pthreads*)malloc(sizeof(struct List_and_Pthreads));
    if(LaP == NULL){
        fprintf(stderr, "memory LaP error\n");
        return NULL;
    }
    LaP->list = list;
    LaP->size_pthread = size_pthread;
    LaP->pthreads = (pthread_t*)malloc(sizeof(pthread_t)*size_pthread);
    if(LaP->pthreads == NULL){
        fprintf(stderr, "memory Pthread error\n");
        return NULL;
    }
    for(i = 0; i < size_pthread; ++i){
        code = pthread_create(&((LaP->pthreads)[i]), NULL, thread_body, list);
        if (code != 0) {
            char bufer_err[256];
            strerror_r(code, bufer_err, sizeof bufer_err);
            fprintf(stderr, "creating thread: %s\n", bufer_err);
            return NULL;
        }
    }
    return LaP;

}

void destroy_List_and_Pthreads(struct List_and_Pthreads* LaP) {
    int i;
    for(i = 0; i < LaP->size_pthread; ++i){
        pthread_cancel((LaP->pthreads)[i]);
        pthread_join((LaP->pthreads)[i], NULL);
    }
    free(LaP->pthreads);
    free(LaP);
}

int foo(int return_value, char* mail){
    if(return_value){
	fprintf(stderr, "error return_value = %d, %s\n", return_value, mail);
    } else {

    }
    return return_value;
}