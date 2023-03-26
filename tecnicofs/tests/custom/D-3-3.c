#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


#define MAX_SIZE INODE_SIZE_AVAILABLE

void* fn(void* arg);

int main() {

    assert(tfs_init() == 0);

    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;

    if (pthread_create(&tid1, NULL, fn, NULL) != 0) {
        fprintf(stderr, "Error creating thread 1\n");
    }

    if (pthread_create(&tid2, NULL, fn, NULL) != 0) {
        fprintf(stderr, "Error creating thread 2\n");
    }

    if (pthread_create(&tid3, NULL, fn, NULL) != 0) {
        fprintf(stderr, "Error creating thread 3\n");
    }

    if (pthread_join(tid1, NULL) != 0) {
        fprintf(stderr, "Error in thread 1\n");
    }

    if (pthread_join(tid2, NULL) != 0) {
        fprintf(stderr, "Error in thread 2\n");
    }

    if (pthread_join(tid3, NULL) != 0) {
        fprintf(stderr, "Error in thread 3\n");
    }

    int file = tfs_open("/testfile", 0);
    assert(file != -1);

    char buffer[MAX_SIZE];
    char bufferAns[MAX_SIZE];
    memset(bufferAns, 'a', MAX_SIZE);

    ssize_t r = tfs_read(file, buffer, MAX_SIZE);
    assert(r == MAX_SIZE);
    assert(memcmp(buffer, bufferAns, MAX_SIZE) == 0);

    printf("Successful test.\n");

    return 0;
}

void* fn(void* arg) {
    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    char buffer[MAX_SIZE];
    memset(buffer,'a', MAX_SIZE);
    ssize_t r = tfs_write(file, buffer, MAX_SIZE);
    assert(r == MAX_SIZE);

    tfs_close(file);

    // So the compiler does not complain about not using the argument
    if (1) {file = *(int*)arg;} 

    return NULL;
}