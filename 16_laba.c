#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define SIZEBUF 30
#define TIMESLEEP 5

struct Vertex {
    char* string;
    struct Vertex* next;
};

struct List {
    struct Vertex* head;
    struct Vertex* tail;
    unsigned int size;
};

void* thread_body(void* param);
void sorting_list(struct List* list);
void add_vertex_in_list(struct Vertex* vertex, struct List* lis);
void print_list(struct List list);
void destroy_list(struct List* list);
int compare(char* str1, char* str2);
void swap_neighbours(struct List* list, unsigned ver);
struct Vertex* init_Vertex(char* string);
struct List* init_List();

pthread_mutex_t accesslock;

int main (int argc, char *argv[]) {
    pthread_t thread;
    struct List* list;
    int code;
    int i;

    pthread_mutex_init (&accesslock, NULL);
    list = init_List();

    code = pthread_create(&thread, NULL, thread_body, list);
    if (code != 0) {
        char bufer_err[256];
        strerror_r(code, bufer_err, sizeof bufer_err);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], bufer_err);
        exit(1);
    }
    while(1){
        char* buf = (char*)malloc(sizeof(char)*SIZEBUF);
        fgets(buf, SIZEBUF, stdin);
        pthread_mutex_lock(&accesslock);
        if(buf[0] == '\n'){
            print_list(*list);
        } else if(buf[0] == '`')
            break;
        else {
            struct Vertex* ver = init_Vertex(buf);
            add_vertex_in_list(ver, list);
        }
        pthread_mutex_unlock(&accesslock);
    }

    pthread_cancel(thread);
    pthread_join(thread, NULL);
    destroy_list(list);
    pthread_mutex_destroy(&accesslock);
    return 0;
}


void * thread_body(void *param) {
    while(1){
        sleep(TIMESLEEP);
        pthread_mutex_lock(&accesslock);
        sorting_list((struct List*)param);
        pthread_mutex_unlock(&accesslock);
    }
    return NULL;
}

void sorting_list (struct List* list) {
    int i,j;
    int size = list->size;
    struct Vertex* ver_i, * ver_j;
    if(list->size < 2){
        return;
    }
    for(i = 0; i < size; ++i){
        ver_i = list->head;
        ver_j = list->head->next;
        for(j = 0; j < size - i - 1; ++j){
            if(compare(ver_i->string, ver_j->string) > 0){
                swap_neighbours(list, j);
//                ver_i = ver_i->next;
                if(ver_i->next != NULL){
                    ver_j = ver_i->next;
                }
            } else {
                ver_i = ver_i->next;
                if(ver_i != NULL){
                    ver_j = ver_j->next;
                }
            }
        }
    }
//    sleep(5);
}

void add_vertex_in_list (struct Vertex *vertex, struct List* list) {
    if(!list->size){
        list->head = vertex;
        list->tail = vertex;
        list->size = 1;
    } else {
        list->tail->next = vertex;
        list->tail = vertex;
        ++list->size;
    }
}

void print_list(struct List list){
    struct Vertex* var;
    fprintf(stderr, "\n************PRINT_LIST***********\n");
    if(!list.size){
        return;
    }
    if(list.size == 1){
        fprintf(stderr, "%s\n", list.head->string);
        return;
    }
    for(var = list.head; var != NULL; var = var->next){
        fprintf(stderr, "%s\n", var->string);
    }
}

void destroy_list(struct List* list) {
    if(!list->size){
        free(list);
        return;
    }
    if(list->size == 1){
        free(list->head);
        free(list);
        return;
    }
    struct Vertex* var1 = list->head;
    struct Vertex* var2 = list->head->next;
    while(var2 != NULL){
        free(var1);
        var1 = var2;
        var2 = var2->next;
    }
    free(var1);
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
void swap_neighbours(struct List* list, unsigned ver) {
    int i;
    struct Vertex* var = list->head;
    struct Vertex *a, *b, *c;
    if(ver == 0){
        struct Vertex *var2, *var3;
        var2 = list->head->next;
        var3 = list->head->next->next;
        list->head->next->next = list->head;
        list->head->next = var3;
        list->head = var2;
        if(list->size == 2){
            list->tail = list->head->next;
        }
        return;
    }
    for(i = 0; i < ver - 1; ++i){
        var = var->next;
    }
    a = var->next;
    b = var->next->next;
    c = var->next->next->next;
    b->next = a;
    a->next = c;
    var->next = b;
    if(ver == list->size - 2){
        list->tail = a;
    }
}

struct Vertex* init_Vertex(char* string) {
    struct Vertex* ver = (struct Vertex*)malloc(sizeof(struct Vertex));
    ver->string = string;
    ver->next = NULL;
    return ver;
}

struct List* init_List() {
    struct List* list = (struct List*)malloc(sizeof(struct List));
    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    return list;
}