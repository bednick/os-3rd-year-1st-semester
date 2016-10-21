#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


struct Strings{
    int size;
    char** strings;
};

void * thread_body(void * param) {
    if(param != NULL){
        int i;
        for(i = 0; i < (*(struct Strings*)(param)).size; ++i){
            fprintf(stderr,"%s\n",(*(struct Strings*)(param)).strings[i]);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread1, thread2, thread3, thread4;
    int code;
    int i;
    struct Strings str1, str2, str3, str4;
    
    str1.size = 2;
    str1.strings = (char**)malloc(sizeof(char*) * (str1.size));
    str1.strings[0] = "str1.1";
    str1.strings[1] = "str1.2";
    code = pthread_create(&thread1, NULL, &thread_body, &str1);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    str2.size = 2;
    str2.strings = (char**)malloc(sizeof(char*) * (str2.size));
    str2.strings[0] = "str2.1";
    str2.strings[1] = "str2.2";
    code = pthread_create(&thread2, NULL, thread_body, &str2);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    str3.size = 2;
    str3.strings = (char**)malloc(sizeof(char*) * (str3.size));
    str3.strings[0] = "str3.1";
    str3.strings[1] = "str3.2";
    code = pthread_create(&thread3, NULL, thread_body, &str3);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    str4.size = 2;
    str4.strings = (char**)malloc(sizeof(char*) * (str4.size));
    str4.strings[0] = "str4.1";
    str4.strings[1] = "str4.2";
    code = pthread_create(&thread4, NULL, thread_body, &str4);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    pthread_join(thread1, NULL);
    free(str1.strings);
    pthread_join(thread2, NULL);
    free(str2.strings);
    pthread_join(thread3, NULL);
    free(str3.strings);
    pthread_join(thread4, NULL);
    free(str4.strings);
    return 0;

}