#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/*
 Several threads try to write their letter (truncating the file). In
 end, the file must only contain one letter. This tests the inode lock.
 */
#define BUFFER_SIZE 100
#define THREAD_COUNT 20
#define FILE_NAME "/testfile"
#define START_CHAR 'a'

void* fn(void* arg);

int main() {

    assert(tfs_init() == 0);

    // Create file
    int file_create = tfs_open(FILE_NAME, TFS_O_CREAT);
    assert(file_create != -1);

    // Create threads and wait
    pthread_t tid[THREAD_COUNT];

    char my_char = START_CHAR;
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_create(&tid[i], NULL, fn, &my_char)) {
            return -1;
        }
        my_char++;
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        if (pthread_join(tid[i], NULL) != 0) {
            return -1;
        }
    }

    // Read result
    int file_read = tfs_open(FILE_NAME, 0);
    char my_text[BUFFER_SIZE];
    assert(tfs_read(file_read, my_text, BUFFER_SIZE) == BUFFER_SIZE);

    // Set expected answer
    char first_char = my_text[0];
    char expected_ans[BUFFER_SIZE];
    memset(expected_ans, first_char, BUFFER_SIZE);

    // Check result
    assert(memcmp(my_text, expected_ans, BUFFER_SIZE) == 0);

    printf("Successful test.\n");
    return 0;
}

void* fn(void* arg) {

    sleep((unsigned int) rand() % 5);

    char my_char = *((char*) arg);
    int file = tfs_open(FILE_NAME, TFS_O_TRUNC);
    assert(file != -1);

    // Create buffer with given char
    char* buffer = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    memset(buffer, my_char, BUFFER_SIZE);

    // Write the buffer to file
    assert(tfs_write(file, buffer, BUFFER_SIZE) == BUFFER_SIZE);

    assert(tfs_close(file) == 0);

    return NULL;
}