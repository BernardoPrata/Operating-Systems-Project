#include "operations.h"
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define WRITE_SIZE 50

#define N 1000
// Verifica tfs_write e tfs_read em concurrencia em  ficheiros abertos
// diferentes diferentes Coexistencia de leitura simultanea--> Testa acesso ao
// mesmo i-node para leitura e leitura/escrita

void *fn1(void *arg);
void *fn2(void *arg);
void *fn3(void *arg);

int main() {

    pthread_t tid[N];
    char buffer[WRITE_SIZE];
    char bufferRead[WRITE_SIZE];

    assert(tfs_init() != -1);

    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    memset(buffer, 'P', WRITE_SIZE);
    ssize_t r = tfs_write(file, buffer, WRITE_SIZE);
    tfs_close(file);

    for (int i = 0; i < N; i++) {
        if (i % 3 == 0) {
            pthread_create(&tid[i], NULL, fn1, NULL);
        } else if (i % 2 == 0) {
            pthread_create(&tid[i], NULL, fn2, NULL);
        } else {
            pthread_create(&tid[i], NULL, fn3, NULL);
        }
    }
    for (int i = 0; i < N; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            fprintf(stderr, "Error in thread %d\n", i);
        }
    }

    file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    r = tfs_read(file, bufferRead, WRITE_SIZE);
    bufferRead[r] = '\0';
    assert(r == WRITE_SIZE);

    // Verify if result equals one of the possible solutions
    char possibleBuffer[WRITE_SIZE];
    memset(possibleBuffer, 'A', WRITE_SIZE);
    if (memcmp(possibleBuffer, bufferRead, WRITE_SIZE) == 0) {
        printf("Successful Test-A\n");
    }
    memset(possibleBuffer, 'Z', WRITE_SIZE);
    if (memcmp(possibleBuffer, bufferRead, WRITE_SIZE) == 0) {
        printf("Successful Test-Z\n");
    }
    tfs_destroy();
    // printf("Texto:\n%s\n", bufferRead);
    return 0;
}

void *fn1(void *arg) {
    // Reads content
    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    char buffer[WRITE_SIZE];
    ssize_t r = tfs_read(file, buffer, WRITE_SIZE);
    assert(r == WRITE_SIZE);

    tfs_close(file);

    if (0) { file = *((int*) arg);}

    return NULL;
}

void *fn2(void *arg) {
    // Fills content with Z
    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    char buffer[WRITE_SIZE];
    memset(buffer, 'Z', WRITE_SIZE);

    ssize_t r = tfs_write(file, buffer, WRITE_SIZE);
    assert(r == WRITE_SIZE);
    tfs_close(file);

    if (0) { file = *((int*) arg);}

    return NULL;
}
void *fn3(void *arg) {
    // Fills content with A
    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    char buffer[WRITE_SIZE];
    memset(buffer, 'A', WRITE_SIZE);

    ssize_t r = tfs_write(file, buffer, WRITE_SIZE);
    assert(r == WRITE_SIZE);
    tfs_close(file);

    if (0) { file = *((int*) arg);}

    return NULL;
}