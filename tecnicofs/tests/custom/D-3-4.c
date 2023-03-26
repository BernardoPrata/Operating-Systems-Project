#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define ALPHA_LENGTH 6// abcdef
#define ALPHA_COUNT 20
#define TEXT_LENGTH ALPHA_COUNT * ALPHA_LENGTH
#define end_str(string, len) string[len] = '\0'

void* fn(void* arg);

int main() {

    assert(tfs_init() == 0);

    char* abc1 = "abcdef";
    char* buffer1 = (char*) malloc(sizeof(char) * (TEXT_LENGTH + 1));
    for (int i = 0; i < ALPHA_COUNT; i++) {
        memcpy(buffer1 + i * ALPHA_LENGTH, abc1, ALPHA_LENGTH);
    }

    char* abc2 = "ghijkl";
    char* buffer2 = (char*) malloc(sizeof(char) * (TEXT_LENGTH + 1));
    for (int i = 0; i < ALPHA_COUNT; i++) {
        memcpy(buffer2 + i * ALPHA_LENGTH, abc2, ALPHA_LENGTH);
    }

    char* abc3 = "mnopqr";
    char* buffer3 = (char*) malloc(sizeof(char) * (TEXT_LENGTH + 1));
    for (int i = 0; i < ALPHA_COUNT; i++) {
        memcpy(buffer3 + i * ALPHA_LENGTH, abc3, ALPHA_LENGTH);
    }

    end_str(buffer1, TEXT_LENGTH);
    end_str(buffer2, TEXT_LENGTH);
    end_str(buffer3, TEXT_LENGTH);

    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    assert(tfs_close(file) == 0);

    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;

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
    assert(file != -1);

    char real_text[TEXT_LENGTH];
    ssize_t r = tfs_read(file, real_text, TEXT_LENGTH);
    assert(r == TEXT_LENGTH);
    assert(memcmp(real_text, buffer1, TEXT_LENGTH) == 0 ||
           memcmp(real_text, buffer2, TEXT_LENGTH) == 0 ||
           memcmp(real_text, buffer3, TEXT_LENGTH) == 0 );

    free(buffer1);
    free(buffer2);
    free(buffer3);

    printf("Successful test.\n");

    return 0;
}

void* fn(void* arg) {
    int file = tfs_open("/testfile", TFS_O_TRUNC);
    assert(file != -1);

    char* buffer = (char*) arg;
    ssize_t r = tfs_write(file, buffer, TEXT_LENGTH);
    assert(r == TEXT_LENGTH);

    tfs_close(file);

    return NULL;
}