#include "operations.h"
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define WRITE_SIZE MAX_SIZE
#define N 1000

void *fn(void *arg);
// Testa vários tfs_read em concurrencia com ficheiros abertos diferentes
// Leitura do conteudo do ficheiro no fim
// --> Testa acesso ao mesmo i-node para leitura
char bufferIn[WRITE_SIZE];

int main() {
    pthread_t tid[N];
    // Buffer
    memset(bufferIn, 'A', WRITE_SIZE);

    assert(tfs_init() != -1);

    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);
    ssize_t r = tfs_write(file, bufferIn, WRITE_SIZE);
    assert(r == WRITE_SIZE);

    tfs_close(file);

    for (int i = 0; i < N; i++) {
        if (pthread_create(&tid[i], NULL, fn, NULL) != 0)
            fprintf(stderr, "Error creating thread 1\n");
    }
    for (int i = 0; i < N; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            fprintf(stderr, "Error in thread %d\n", i);
        }
    }

    char bufferOut[WRITE_SIZE];
    file = tfs_open("/testfile", TFS_O_CREAT);
    r = tfs_read(file, bufferOut, WRITE_SIZE);
    bufferOut[r] = '\0';
    if (memcmp(bufferIn, bufferOut, WRITE_SIZE) == 0) {
        printf("Successful Test.\n");
        return 0;
    }
    printf("GAME OVER.\n");
    return 0;
}

void *fn(void *arg) {
    int file = tfs_open("/testfile", 0);
    assert(file != -1);

    char bufferOut[WRITE_SIZE];
    ssize_t r = tfs_read(file, bufferOut, WRITE_SIZE);
    assert(r == WRITE_SIZE);
    assert(memcmp(bufferIn, bufferOut, WRITE_SIZE) == 0);

    assert(tfs_close(file) == 0);

    // So the compiler does not complain
    if (1) file = *((int*) arg);

    return NULL;
}