#include "fs/operations.h"
#include <assert.h>
#include <string.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define WRITE_SIZE MAX_SIZE
#define N 100

void *fn1(void *args);
void *fn2(void *args);
// Testa vários  tfs_write em concurrencia  com ficheiros abertos diferentes
// Leitura do conteudo do ficheiro no fim
// --> Testa acesso ao mesmo i-node

int main() {

    assert(tfs_init() != -1);
    char bufferOut[WRITE_SIZE];
    pthread_t tid[N];
    for (int i = 0; i < N; i++) {
        if (i % 2 != 0) {
            if (pthread_create(&tid[i], NULL, fn1, NULL) != 0)
                fprintf(stderr, "Error creating thread 1\n");
        } else if (pthread_create(&tid[i], NULL, fn2, NULL) != 0) {
            fprintf(stderr, "Error creating thread 2\n");
        }
    }
    for (int i = 0; i < N; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            fprintf(stderr, "Error in thread %d\n", i);
        }
    }

    int fileOut = tfs_open("/testfile", 0);
    assert(fileOut != -1);

    ssize_t r = tfs_read(fileOut, bufferOut, MAX_SIZE);
    assert(r == MAX_SIZE);

    assert(tfs_close(fileOut) == 0);
    char ans1[MAX_SIZE];
    char ans2[MAX_SIZE];
    memset(ans1, 'A', MAX_SIZE);
    memset(ans2, 'Z', MAX_SIZE);
    assert(memcmp(ans1, bufferOut, MAX_SIZE) == 0 || memcmp(ans2, bufferOut, MAX_SIZE) == 0);

    printf("Successful test.\n");

    tfs_destroy();
    
    return 0;
}
void *fn1(void *args) {

    char bufferIn[WRITE_SIZE];
    memset(bufferIn, 'A', sizeof(bufferIn));
    int fileIn = tfs_open("/testfile", TFS_O_CREAT);
    assert(fileIn != -1);

    ssize_t r = tfs_write(fileIn, bufferIn, WRITE_SIZE * sizeof(char));
    assert(r == WRITE_SIZE);

    tfs_close(fileIn);

    // So the compiler does not complain
    if (1) fileIn = *((int*) args);

    return 0;
}
void *fn2(void *args) {

    char bufferIn[WRITE_SIZE];
    memset(bufferIn, 'Z', sizeof(bufferIn));
    int fileIn = tfs_open("/testfile", TFS_O_CREAT);
    assert(fileIn != -1);

    ssize_t r = tfs_write(fileIn, bufferIn, WRITE_SIZE * sizeof(char));
    assert(r == WRITE_SIZE);

    tfs_close(fileIn);

    // So the compiler does not complain
    if (1) fileIn = *((int*) args);

    return 0;
}
