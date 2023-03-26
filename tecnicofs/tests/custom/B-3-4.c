#include "operations.h"
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#define MAX_SIZE INODE_SIZE_AVAILABLE
#define WRITE_SIZE 2
#define N 100

// Leitura e Escrita em Concurrência no mesmo ficheiro aberto
// Escreve  tfs_write, em modo append ,concurrencia no mesmo ficheiro.
// Misses A good verification case, the easier way until now is to check the
// output.

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

    tfs_close(file);
    file = tfs_open("/testfile", TFS_O_CREAT);
    if (writesNumber > 0) {

        char bufferRead[WRITE_SIZE * writesNumber];
        ssize_t r = tfs_read(file, bufferRead, (unsigned int) (WRITE_SIZE * writesNumber));
        bufferRead[r] = '\0';
        assert((WRITE_SIZE * writesNumber) % 2 == 0);

        assert(r == WRITE_SIZE * writesNumber);
        // printf("BufferRead leu string de tamanho %ld :\n%s\n\n",
        //      strlen(bufferRead), bufferRead);
    }
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
    // Fills content with Z
    int file = *((int *)arg);
    assert(file != -1);

    char buffer[WRITE_SIZE];
    memset(buffer, 'Z', WRITE_SIZE);

    ssize_t r = tfs_write(file, buffer, WRITE_SIZE);
    assert(r == WRITE_SIZE);
    pthread_mutex_lock(&lockUpdate);
    writesNumber++;
    pthread_mutex_unlock(&lockUpdate);

    return NULL;
}
