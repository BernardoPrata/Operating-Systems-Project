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

    pthread_t tid1;
    pthread_t tid2;
    pthread_t tid3;

    assert(tfs_init() == 0);

    char* abcdef = "abcdef";
    char* buffer = (char*) malloc(sizeof(char) * (TEXT_LENGTH + 1));
    for (int i = 0; i < ALPHA_COUNT; i++) {
        memcpy(buffer + i * ALPHA_LENGTH, abcdef, ALPHA_LENGTH);
    }

    end_str(buffer, TEXT_LENGTH);

    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    ssize_t r = tfs_write(file, buffer, TEXT_LENGTH);
    assert(r == TEXT_LENGTH);

    if (pthread_create(&tid1, NULL, fn, buffer) != 0) {
        fprintf(stderr, "Error creating thread 1\n");
    }

    if (pthread_create(&tid2, NULL, fn, buffer) != 0) {
        fprintf(stderr, "Error creating thread 2\n");
    }

    if (pthread_create(&tid3, NULL, fn, buffer) != 0) {
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

    printf("Succesful test.\n");

    free(buffer);

    return 0;
}

void* fn(void* arg) {
    int file = tfs_open("/testfile", TFS_O_CREAT);
    assert(file != -1);

    char* buffer_correct = (char*) arg;
    char buffer_mine[TEXT_LENGTH + 1];

    ssize_t r = tfs_read(file, buffer_mine, TEXT_LENGTH);
    assert(r == TEXT_LENGTH);
    assert(memcmp(buffer_correct, buffer_mine, TEXT_LENGTH) == 0);

    tfs_close(file);

    return NULL;
}