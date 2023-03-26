#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 100
#define FILE_NAME "/testfile"
#define THREADS_PER_LETTER 100

void* fn_a(void* arg);
void* fn_b(void* arg);

// TODO: Add randomized sleep

/**
 * Test if locks on open file entries work.
 * Program creates threads and they simoultaneously try to write 
 * through the same file handle (appending text to it). If the locks 
 * weren't in place, unwanted behaviour would happen (the strings would 
 * be intercalated).
 */
int main() {

    assert(tfs_init() == 0);

    // Create file
    int file_create = tfs_open(FILE_NAME, TFS_O_CREAT);
    assert(file_create != -1);
    assert(tfs_close(file_create) == 0);

    // Open in append mode
    int file = tfs_open(FILE_NAME, TFS_O_APPEND);
    assert(file != -1);

    // Create threads and wait for execution
    pthread_t tid_a[THREADS_PER_LETTER];
    pthread_t tid_b[THREADS_PER_LETTER];

    for (int i = 0; i < THREADS_PER_LETTER; i++) {
        if (pthread_create(&tid_a[i], NULL, fn_a, &file) != 0) {
            return -1;
        }
    }

    for (int i = 0; i < THREADS_PER_LETTER; i++) {
        if (pthread_create(&tid_b[i], NULL, fn_b, &file) != 0) {
            return -1;
        }
    }

    for (int i = 0; i < THREADS_PER_LETTER; i++) {
        if (pthread_join(tid_a[i], NULL) != 0) {
            return -1;
        }
    }

    for (int i = 0; i < THREADS_PER_LETTER; i++) {
        if (pthread_join(tid_b[i], NULL) != 0) {
            return -1;
        }
    }

    // Check result
    char *sample_a = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    char *sample_b = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    memset(sample_a, 'a', BUFFER_SIZE);
    memset(sample_b, 'b', BUFFER_SIZE);

    int file_read = tfs_open(FILE_NAME, 0);
    char text_read[BUFFER_SIZE];

    int a_count = 0;
    int b_count = 0;

    for (int i = 0; i < THREADS_PER_LETTER * 2; i++) {

        assert(tfs_read(file_read, text_read, BUFFER_SIZE) == BUFFER_SIZE);     

        assert(
            memcmp(sample_a, text_read, BUFFER_SIZE) == 0 ||
            memcmp(sample_b, text_read, BUFFER_SIZE) == 0
        );

        if (text_read[0] == 'a') {
            a_count++;
        }
        else {
            b_count++;
        }
    }

    assert(a_count == b_count);
    assert(a_count == THREADS_PER_LETTER);

    printf("Successful test.\n");

    return 0;
}

void* fn_a(void* arg) {

    sleep((unsigned int) rand() % 5);

    int file = *((int*) arg);

    // Creates buffer to write
    char *buffer_a = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    memset(buffer_a, 'a', BUFFER_SIZE);

    // Do the writing
    assert(tfs_write(file, buffer_a, BUFFER_SIZE) == BUFFER_SIZE);

    return NULL;
}

void* fn_b(void* arg) {

    sleep((unsigned int) rand() % 5);

    int file = *((int*) arg);

    // Creates buffer to write
    char *buffer_b = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    memset(buffer_b, 'b', BUFFER_SIZE);

    // Do the writing
    assert(tfs_write(file, buffer_b, BUFFER_SIZE) == BUFFER_SIZE);

    return NULL;
}

