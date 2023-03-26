#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define COUNT_PER_CHAR 9728

int main() {

    assert(tfs_init() != -1);

    char bufferResult[MAX_SIZE];
    char bufferInitial[MAX_SIZE];
    char fill_char = 'a';

    for (size_t i = 0; i < MAX_SIZE; i++) {
        if (i % COUNT_PER_CHAR == 0 && i > 0) fill_char++;
        bufferResult[i] = fill_char;
    }

    int dummy = tfs_open("/testfile", TFS_O_CREAT);
    assert(dummy != -1);

    int file = tfs_open("/testfile", TFS_O_APPEND);
    assert(file != -1);


    fill_char = 'a';
    for (int i = 0; i < MAX_SIZE / COUNT_PER_CHAR; i++) {
        memset(bufferInitial, fill_char, MAX_SIZE);
        ssize_t r = tfs_write(file, bufferInitial, COUNT_PER_CHAR);
        assert(r == COUNT_PER_CHAR);
        fill_char++;
    }

    memset(bufferInitial, '\0', MAX_SIZE);

    int file2 = tfs_open("/testfile", 0);
    assert(file2 != -1);

    ssize_t r = tfs_read(file2, bufferInitial, MAX_SIZE);
    assert(r == MAX_SIZE);

    assert(strncmp(bufferInitial, bufferResult, MAX_SIZE) == 0);

    printf("Successful test.\n");

    return 0;
}