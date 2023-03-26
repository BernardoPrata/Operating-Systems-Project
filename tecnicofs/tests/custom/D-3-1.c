#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE

void* fn1(void* arg);
void* fn2(void* arg);
void* fn3(void* arg);

int main() {

    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;
    assert(tfs_init()!=-1);
    if (pthread_create(&tid1, NULL, fn1, NULL) != 0) {
        fprintf(stderr, "Error creating thread 1\n");
    }

    if (pthread_create(&tid2, NULL, fn2, NULL) != 0) {
        fprintf(stderr, "Error creating thread 2\n");
    }

    if (pthread_create(&tid3, NULL, fn3, NULL) != 0) {
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


    int file1 = tfs_open("/testfile1", 0);
    int file2 = tfs_open("/testfile2", 0);
    int file3 = tfs_open("/testfile3", 0);

    assert(file1 != -1);
    assert(file2 != -1);
    assert(file3 != -1);

    printf("Successful test.\n");

    return 0;
}

void* fn1(void* arg) {
    int file = tfs_open("/testfile1", TFS_O_CREAT);
    assert(file != -1);

    tfs_close(file);

    // So the compiler does not complain about not using the argument
    if (0) {file = *(int*)arg;} 
    
    return NULL;
}

void* fn2(void* arg) {
    int file = tfs_open("/testfile2", TFS_O_CREAT);
    assert(file != -1);


    tfs_close(file);
    
    if (0) {file = *(int*)arg;}

    return NULL;
}

void* fn3(void* arg) {
    int file = tfs_open("/testfile3", TFS_O_CREAT);
    assert(file != -1);


    tfs_close(file);

    if (0) {file = *(int*)arg;}

    return NULL;
}
