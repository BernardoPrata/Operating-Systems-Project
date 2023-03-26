#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define TRIALS 100
#define LENGTH 10000


/**
 * Several threads truncate and write into a file. This shows how the locks
 * on the inodes are enought to protect the matching data blocks. (if 
 * there was any sharing of data blocks, the blocks would not have only one
 * letter but several).
 */

void* truncate_and_write(void* arg);

int main() {

    assert(tfs_init() != -1);

    // Create all files
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        char* filename = (char*) malloc(sizeof(char) * 3);
        filename[0] = '/';
        filename[1] = (char) i + 97;
        filename[2] = '\0';
        int fhandle = tfs_open(filename, TFS_O_CREAT);
        assert(fhandle != -1);
        assert(tfs_close(fhandle) == 0);
    }

    pthread_t tids[MAX_OPEN_FILES];

    for (int i = 0; i < TRIALS; i++) {
        // Round of truncate + write

        for (int j = 0; j < MAX_OPEN_FILES; j++) {
            char* filename = (char*) malloc(sizeof(char) * 3);
            filename[0] = '/';
            filename[1] = (char) j + 97;
            filename[2] = '\0';
            if (pthread_create(&tids[j], NULL, truncate_and_write, (void*) filename) != 0) {
                printf("here\n");
                return -1;
            }
        }

        // Wait for all threads to end
        for (int j = 0; j < MAX_OPEN_FILES; j++) {
            if (pthread_join(tids[j], NULL) != 0) {
                assert(1);
                return -1;
            }
        }

        char buffer_read[LENGTH];
        char expected_buffer[LENGTH];

        for (int j = 0; j < MAX_OPEN_FILES; j++) {
            char* filename = (char*) malloc(sizeof(char) * 3);
            filename[0] = '/';
            filename[1] = (char) j + 97;
            filename[2] = '\0';

            int fhandle = tfs_open(filename, 0);
            assert(fhandle != -1);

            assert(tfs_read(fhandle, buffer_read, LENGTH)
                == LENGTH);

            // Get expected answer
            char my_char = buffer_read[0];
            memset(expected_buffer, my_char, LENGTH);

            // Check if read value is correct
            assert(memcmp(buffer_read, expected_buffer, LENGTH) == 0);

            assert(fhandle != -1);
            assert(tfs_close(fhandle) != -1);
        }
    }

    printf("Successful test.\n");

    return 0;
}

void* truncate_and_write(void* arg) {

    // Open file
    char* filename = (char*) arg;
    int fhandle = tfs_open(filename, TFS_O_TRUNC);
    assert(fhandle != -1);

    // Get string to write
    char my_char = (char) (rand() % 20) + 97;
    char to_write[LENGTH];
    memset(to_write, my_char, LENGTH);

    // Write
    ssize_t r = tfs_write(fhandle, to_write, LENGTH);
    assert(r == LENGTH);

    assert(tfs_close(fhandle) == 0);

    return NULL;
}