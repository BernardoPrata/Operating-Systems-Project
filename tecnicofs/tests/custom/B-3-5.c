#include "operations.h"
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#define MAX_SIZE INODE_SIZE_AVAILABLE
#define WRITE_SIZE 2
#define N 3000

// Escrita e fechar o mesmo ficheiro aberto

void *fn1(void *arg);
void *fn2(void *arg);
int writesNumber = 0;
pthread_mutex_t lockUpdate = PTHREAD_MUTEX_INITIALIZER;
int main() {

    pthread_t tid[N];

    assert(tfs_init() != -1);

    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    for (int i = 0; i < N; i++) {
        if (i % 5 == 0) {
            pthread_create(&tid[i], NULL, fn1, &file);
        } else {
            pthread_create(&tid[i], NULL, fn2, &file);
        }
    }


    for (int i = 0; i < N; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            fprintf(stderr, "Error in thread %d\n", i);
        }
    }

    file = tfs_close(file);
    assert(file == -1); // -1 significa que ja está fechado
    file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file == 0); // 0 significa que é o 1º file a ser criado
    printf("Successful Test.\n");
    tfs_destroy();
    return 0;
}

void *fn1(void *arg) {
    // Fills content with A
    int file = *((int *)arg);
    assert(file != -1);

    char buffer[WRITE_SIZE];
    memset(buffer, 'A', WRITE_SIZE);

    ssize_t r = tfs_write(file, buffer, WRITE_SIZE);

    assert(r == WRITE_SIZE);
    pthread_mutex_lock(&lockUpdate);
    writesNumber++;
    pthread_mutex_unlock(&lockUpdate);
    return NULL;
}
void *fn2(void *arg) {
    // Closes given file
    int file = *((int *)arg);
    file = tfs_close(file);
    assert(file == -1); // 0 é o identificador deste ficheiro

    return NULL;
}
