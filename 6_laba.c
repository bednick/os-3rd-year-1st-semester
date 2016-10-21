#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

struct Vertex {
    pthread_t* pthread;
    char* string_1;
    char* string_2;
    struct Vertex* next;
};

struct Queue {
    pthread_mutex_t lock;
    struct Vertex* head;
    struct Vertex* tail;
};

struct Queue Queue;

int create_thread(void *(*start_routine) (void *), char *theDir_1, char *theDir_2);
void* process_directory( void *vertex );
void* copy_file(void *nameFile);

void queue_push(struct Queue* queue);               // Уничтожить первый элемент в очереди
struct Vertex* queue_add (struct Queue*, char* string_1, char* string_2); // Вносит в очередь (Создаёт Vertex), возвращает ссылку Vertex(не создаёт его)

int main( int argc, char **argv )
{
    int i = 0;
    if(argc != 3){
        fprintf(stderr, "error argc != 3 введите 2 параметрf(имя дир)\n");
        return -1;
    }
    pthread_mutex_init(&(Queue.lock));
    Queue.head = NULL;
    Queue.tail = NULL;
    if( strstr( argv[2], argv[1] ) != NULL ){
        fprintf(stderr, "Нельзя копировать в собственный подкаталог\n");
        return -2;
    }
    if(create_thread(process_directory, argv[1], argv[2])){
        fprintf(stderr, "error create thread\n");
        return -3;
    }
    while(Queue.head != NULL){
        pthread_join(*(Queue.head->pthread), NULL);
        queue_push(&Queue);
    }
    pthread_mutex_destroy(&(Queue.lock));
}

int create_thread(void *(*start_routine) (void *), char *theDir_1, char *theDir_2){
    int code;
    struct Vertex* vertex = queue_add(&Queue, theDir_1, theDir_2);
    if(vertex == NULL){
        fprintf(stderr, "error memomy\n");
        return -1;
    }
    code = pthread_create(vertex->pthread, NULL, start_routine, vertex);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "creating thread: %s\n", buf);
    }
    return code;
}

void* process_directory( void * vertex ) {
    DIR *dir = NULL;
    char* theDir = ((struct Vertex*)vertex)->string_1;
    char* theDir_2 = ((struct Vertex*)vertex)->string_2;
    struct dirent entry;
    struct dirent *entryPtr = NULL;
    int retval = 0;

    char pathName[PATH_MAX + 1];
    char pathName_2[PATH_MAX + 1];

    /* открыть указанный каталог, если возможно. */
    dir = opendir( theDir );
    if( dir == NULL ) {
        printf( "Error opening %s: %s\n", theDir, strerror( errno ) );
        pthread_exit(0);
    }

    /* создаём указанный каталог, если возможно. */
    if( mkdir( theDir_2, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ) {
        printf( "Error create %s: %s\n", theDir_2, strerror( errno ) );
        pthread_exit(0);
    } else {
//        fprintf(stderr, "create directory %s\n", theDir_2);
    }
    retval = readdir_r( dir, &entry, &entryPtr );
    while( entryPtr != NULL ) {
        struct stat entryInfo;

        if( ( strncmp( entry.d_name, ".", PATH_MAX ) == 0 ) ||
            ( strncmp( entry.d_name, "..", PATH_MAX ) == 0 ) ) {
            /* Short-circuit the . and .. entries. */
            retval = readdir_r( dir, &entry, &entryPtr );
            continue;
        }
        (void)strncpy( pathName, theDir, PATH_MAX );
        (void)strncat( pathName, "/", PATH_MAX );
        (void)strncat( pathName, entry.d_name, PATH_MAX );

        (void)strncpy( pathName_2, theDir_2, PATH_MAX );
        (void)strncat( pathName_2, "/", PATH_MAX );
        (void)strncat( pathName_2, entry.d_name, PATH_MAX );

        if( lstat( pathName, &entryInfo ) == 0 ) {
            /* вызов stat() был успешным, так что продолжаем. */
            if( S_ISDIR( entryInfo.st_mode ) ) {
    /* каталог */
//                fprintf(stderr, "directory %s\n", pathName);
                create_thread(process_directory, pathName , pathName_2);
            } else if( S_ISREG( entryInfo.st_mode ) ) {
    /* обычный файл */
//                printf( "\t%s has %lld bytes \n", pathName, (long long)entryInfo.st_size );
                create_thread(copy_file, pathName , pathName_2);
            }
        } else {
            printf( "Error statting %s: %s\n", pathName, strerror( errno ) );

        }
        retval = readdir_r( dir, &entry, &entryPtr );
    }
    /* закрытие каталога. */
    (void)closedir( dir );
    return NULL;
}

void* copy_file(void *vertex) {
    char* nameFile_1 = ((struct Vertex*)vertex)->string_1;
    char* nameFile_2 = ((struct Vertex*)vertex)->string_2;
    int file_1, file_2, i;
    size_t size;
    char buf[16384];

    if( ( file_1 = open( nameFile_1,  O_RDONLY ) ) <= 0 ){
        fprintf(stderr, "error open %s\n", nameFile_1);
        pthread_exit(0);
    }
    if( ( file_2 = open( nameFile_2, O_CREAT | O_EXCL | O_WRONLY, S_IWRITE | S_IREAD ) ) <= 0 ){
        fprintf(stderr, "error create %s\n", nameFile_2);
        close(file_1);
        pthread_exit(0);
    }

    while( (size = read( file_1, buf, 16384 )) > 0){
        if(write( file_2, buf, size ) == -1){
            fprintf(stderr, "error write file %s , %s\n", nameFile_2, strerror( errno ) );
        }
        //write( file_2, buf, size );
    }
//    fprintf(stderr, "create file %s\n", nameFile_2);
    close(file_1);
    close(file_2);
    return NULL;
}

void queue_push(struct Queue* queue) {               // Уничтожить первый элемент в очереди
    struct Vertex* head;
    pthread_mutex_lock(&(queue->lock));
    if(queue->head == NULL){
        pthread_mutex_unlock(&(queue->lock));
        return;
    }
    head = queue->head;

    queue->head = queue->head->next;
    if(queue->head == NULL){
        queue->tail == NULL;
    }
    pthread_mutex_unlock(&(queue->lock));

    free(head->pthread);
    free(head->string_1);
    free(head->string_2);
    free(head);
}

struct Vertex* queue_add (struct Queue* queue, char* string_1, char* string_2) { // Вносит в очередь (Создаёт Vertex), возвращает ссылку на pthread_t потока(не создаёт thread)
    struct Vertex* vertex;

    if((vertex = (struct Vertex*)malloc(sizeof(struct Vertex))) == NULL){
        return NULL;
    }
    if((vertex->string_1 = (char*)malloc((strlen(string_1)) + 1)) == NULL){
        free(vertex);
        return NULL;
    }
    if((vertex->string_2 = (char*)malloc((strlen(string_2)) + 1)) == NULL){
        free(vertex->string_1);
        free(vertex);
        return NULL;
    }
    if((vertex->pthread = (pthread_t*)malloc(sizeof(pthread_t))) == NULL){
        free(vertex->string_1);
        free(vertex->string_2);
        free(vertex);
        return NULL;
    }
    (void)strncpy( vertex->string_1, string_1, (strlen(string_1)) + 1 );
    (void)strncpy( vertex->string_2, string_2, (strlen(string_2)) + 1 );
    vertex->next = NULL;

    pthread_mutex_lock(&(queue->lock));
    if(queue->tail == NULL){
        queue->tail = vertex;
        queue->head = vertex;
    } else {
        queue->tail->next = vertex;
        queue->tail = queue->tail->next;
    }
    pthread_mutex_unlock(&(queue->lock));
    return vertex;
}