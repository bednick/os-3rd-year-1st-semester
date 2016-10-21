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
    pthread_mutex_t lock;
};

struct List {
    struct Vertex* head;
    struct Vertex* tail;
    unsigned int size;
    pthread_mutex_t lock_head;
    pthread_mutex_t lock_size; //отвечает за tail и size
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
        fgets(buf, SIZEBUF, stdin);
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
    int i, j, size, flag;
    struct Vertex* begin, *end, *ver1, *ver2;
    pthread_mutex_lock(&(list->lock_size));
    size = list->size;
    pthread_mutex_unlock(&(list->lock_size));

    if(size < 3){
        return;
    }
    for(i = 0; i < size - 1; ++i){
        begin = NULL;
        pthread_mutex_lock(&(list->lock_head));
        pthread_mutex_lock(&(list->head->lock));
        ver1 = list->head;
        pthread_mutex_lock(&(list->head->next->lock));
        ver2 = list->head->next;
        for(j = 0; j < size - i - 1; ++j){
            flag = 0;
            if(j == size - 2){
                pthread_mutex_lock(&(list->lock_size));
            }
            if(compare(ver1->string, ver2->string) > 0){
                swap_neighbours(list, begin);
                flag = 1;
            } else{
            }
            if(j == 0){
                pthread_mutex_unlock(&(list->lock_head));
            } else{
                pthread_mutex_unlock(&(begin->lock));
            }
            if(j == size - 2){
                pthread_mutex_unlock(&(list->lock_size));
            }
            if(flag){
                begin = ver2;
                if(j < size - i - 2){
                    pthread_mutex_lock(&(ver1->next->lock));
                }
                ver2 = ver1->next;
            } else{
                begin = ver1;
                if(j < size - i - 2){
                    pthread_mutex_lock(&(ver2->next->lock));
                }
                ver1 = ver2;
                ver2 = ver2->next;
            }
        }
        pthread_mutex_unlock(&(begin->next->lock));
        pthread_mutex_unlock(&(begin->lock));
    }
}

void add_vertex_in_list (struct Vertex *vertex, struct List* list) {
    int size;
    foo(pthread_mutex_lock(&(list->lock_size)), "add");
    size = list->size;
    foo(pthread_mutex_unlock(&(list->lock_size)), "add");

    if(!size){
        foo(pthread_mutex_lock(&(list->lock_head)), "add");

        foo(pthread_mutex_lock(&(list->lock_size)), "add");
        list->head = vertex;
        list->tail = vertex;
        list->size = 1;
        foo(pthread_mutex_unlock(&(list->lock_size)), "add");
        foo(pthread_mutex_unlock(&(list->lock_head)), "add");

    } else {
        struct Vertex* var;
        foo(pthread_mutex_lock(&(list->tail->lock)), "add");
        foo(pthread_mutex_lock(&(list->lock_size)), "add");
        var = list->tail;
        list->tail->next = vertex;
        list->tail = vertex;
        ++(list->size);
        foo(pthread_mutex_unlock(&(var->lock)), "add");
        foo(pthread_mutex_unlock(&(list->lock_size)), "add");
    }
}

void print_list(struct List* list){
    struct Vertex* var1, * var2;
    foo(pthread_mutex_lock(&(list->lock_size)), "print");
    fprintf(stderr, "\n************PRINT_LIST***********\n");
    if(!list->size){
        foo(pthread_mutex_unlock(&(list->lock_size)), "print");
        fprintf(stderr, "*********************************************\n");
        return;
    }
    if(list->size == 1){
        foo(pthread_mutex_lock(&(list->lock_head)), "print");
        fprintf(stderr, "%s\n", list->head->string);
        foo(pthread_mutex_unlock(&(list->lock_head)), "print");
        foo(pthread_mutex_unlock(&(list->lock_size)), "print");
        fprintf(stderr, "*********************************************\n");
        return;
    }
    foo(pthread_mutex_unlock(&(list->lock_size)), "print");
    foo(pthread_mutex_lock(&(list->lock_head)), "print");
    foo(pthread_mutex_lock(&(list->head->lock)), "print");
    var1 = list->head;
    foo(pthread_mutex_unlock(&(list->lock_head)), "print");
    while(var1 != NULL){
        if(var1->next != NULL){
            foo(pthread_mutex_lock(&(var1->next->lock)), "print");
        }
        var2 = var1->next;
        fprintf(stderr, "%s\n", var1->string);
        foo(pthread_mutex_unlock(&(var1->lock)), "print");
        var1 = var2;
    }
    fprintf(stderr, "*********************************************\n");
}

void destroy_list(struct List* list) {
    if(!list->size){
        pthread_mutex_destroy(&(list->lock_head));
        pthread_mutex_destroy(&(list->lock_size));
        free(list);
        return;
    }
    if(list->size == 1){
        pthread_mutex_destroy(&(list->head->lock));
        free(list->head);
        pthread_mutex_destroy(&(list->lock_head));
        pthread_mutex_destroy(&(list->lock_size));
        free(list);
        return;
    }
    struct Vertex* var1 = list->head;
    struct Vertex* var2 = list->head->next;
    while(var2 != NULL){
        pthread_mutex_destroy(&var1->lock);
        free(var1);
        var1 = var2;
        var2 = var2->next;
    }
    pthread_mutex_destroy(&var1->lock);
    free(var1);
    pthread_mutex_destroy(&list->lock_head);
    pthread_mutex_destroy(&list->lock_size);
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
    pthread_mutex_init(&ver->lock, NULL);
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
    pthread_mutex_init (&list->lock_head, NULL);
    pthread_mutex_init (&list->lock_size, NULL);
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