#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define TEXT_LENGTH 100
#define MAX_SIZE INODE_SIZE_AVAILABLE
#define end_str(string, len) string[len] = '\0'

void* fn(void* arg);

int main() {

    assert(tfs_init() == 0);

    int file = tfs_open("/testfile", TFS_O_CREAT);

    char buffer0[TEXT_LENGTH];
    char *buffer1 = (char*) malloc(sizeof(char) * TEXT_LENGTH);
    char *buffer2 = (char*) malloc(sizeof(char) * TEXT_LENGTH);
    char *buffer3 = (char*) malloc(sizeof(char) * TEXT_LENGTH);

    memset(buffer0, 'a', TEXT_LENGTH);
    memset(buffer1, 'b', TEXT_LENGTH);
    memset(buffer2, 'c', TEXT_LENGTH);
    memset(buffer3, 'd', TEXT_LENGTH);

    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;

    file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    assert(tfs_write(file, buffer0, TEXT_LENGTH) == TEXT_LENGTH);
    assert(tfs_close(file) == 0);

    if (pthread_create(&tid1, NULL, fn, buffer1) != 0) {
        fprintf(stderr, "Error creating thread 1\n");
    }

    if (pthread_create(&tid2, NULL, fn, buffer2) != 0) {
        fprintf(stderr, "Error creating thread 2\n");
    }

    if (pthread_create(&tid3, NULL, fn, buffer3) != 0) {
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

    file = tfs_open("/testfile", 0);
    char ans[TEXT_LENGTH * 4 + 1];

    assert(tfs_read(file, ans, TEXT_LENGTH * 4) == TEXT_LENGTH * 4);
    end_str(ans, TEXT_LENGTH * 4);
    printf("\n%s\n", ans);

    assert(
        memcmp(ans, buffer0, TEXT_LENGTH) && (
        // 1 2 3
        (
            memcmp(ans + TEXT_LENGTH, buffer1, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 2, buffer2, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 3, buffer3, TEXT_LENGTH) 
        ) ||
        // 1 3 2
        (
            memcmp(ans + TEXT_LENGTH, buffer1, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 2, buffer3, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 3, buffer2, TEXT_LENGTH)
        )||
        // 2 1 3
        (
            memcmp(ans + TEXT_LENGTH, buffer2, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 2, buffer1, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 3, buffer3, TEXT_LENGTH)
        )||
        // 2 3 1
        (
            memcmp(ans + TEXT_LENGTH, buffer2, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 2, buffer3, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 3, buffer1, TEXT_LENGTH)
        )||
        // 3 1 2
        (
            memcmp(ans + TEXT_LENGTH, buffer3, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 2, buffer1, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 3, buffer2, TEXT_LENGTH)
        )||
        // 3 2 1
        (
            memcmp(ans + TEXT_LENGTH, buffer3, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 2, buffer2, TEXT_LENGTH) &&
            memcmp(ans + TEXT_LENGTH * 3, buffer1, TEXT_LENGTH)
        )
    ));

    printf("Successful test.\n");

    return 0;
}

void* fn(void* arg) {

    sleep(1);

    int file = tfs_open("/testfile", TFS_O_APPEND);
    assert(file != -1);

    char* buffer = (char*) arg;
    ssize_t r = tfs_write(file, buffer, TEXT_LENGTH);
    assert(r == TEXT_LENGTH);

    tfs_close(file);

    return NULL;
}