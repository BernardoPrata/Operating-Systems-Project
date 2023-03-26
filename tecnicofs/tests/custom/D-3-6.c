#include <pthread.h>
#include <stdio.h>
#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>



#define ALPHA_COUNT 20
#define ALPHA_LENGTH 6// abcdef
#define TEXT_LENGTH ALPHA_COUNT * ALPHA_LENGTH
#define THREAD_COUNT 4000
#define end_str(string, len) string[len] = '\0'

void* fn(void* arg);

int main() {

    assert(tfs_init() == 0);

    char* abc1 = "abcdef";
    char* buffer = (char*) malloc(sizeof(char) * (TEXT_LENGTH + 1));
    for (int i = 0; i < ALPHA_COUNT; i++) {
        memcpy(buffer + i * ALPHA_LENGTH, abc1, ALPHA_LENGTH);
    }

    end_str(buffer, TEXT_LENGTH);

    pthread_t tid[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&tid[i], NULL, fn, buffer) != 0) {
            fprintf(stderr, "Error creating thread 1\n");
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            fprintf(stderr, "Error in thread 1\n");
        }
    }

    int file = tfs_open("/testfile", 0);
    assert(file != -1);

    char ans[TEXT_LENGTH];
    assert(tfs_read(file, ans, TEXT_LENGTH) == TEXT_LENGTH);
    assert(memcmp(ans, buffer, TEXT_LENGTH) == 0);

    free(buffer);
    printf("Successful test.\n");

    return 0;
}

void* fn(void* arg) {
    int file = tfs_open("/testfile", TFS_O_CREAT || TFS_O_TRUNC);
    assert(file != -1);

    char* buffer = (char*) arg;
    ssize_t r = tfs_write(file, buffer, TEXT_LENGTH);
    assert(r == TEXT_LENGTH);

    tfs_close(file);

    return NULL;
}