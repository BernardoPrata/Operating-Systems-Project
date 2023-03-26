#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE

int main() {

    assert(tfs_init() != -1);

    char bufferInitial[MAX_SIZE];
    char bufferResult[MAX_SIZE];

    memset(bufferResult, 'a', MAX_SIZE);
    memset(bufferInitial, 'b', MAX_SIZE);

    int file1 = tfs_open("/testfile", TFS_O_CREAT);
    assert(file1 != -1);

    int file2 = tfs_open("/testfile", 0);
    assert(file2 != -1);

    ssize_t r = tfs_write(file1, bufferResult, LEVEL0_BLOCK_NUM * BLOCK_SIZE);
    assert(r == LEVEL0_BLOCK_NUM * BLOCK_SIZE);
    r += tfs_write(file1, bufferResult, MAX_SIZE - LEVEL0_BLOCK_NUM * BLOCK_SIZE);
    assert(r == MAX_SIZE);

    r = tfs_read(file2, bufferInitial, MAX_SIZE);
    assert(r == MAX_SIZE);

    assert(strncmp(bufferInitial, bufferResult, MAX_SIZE) == 0);

    printf("Successful test.\n");

    return 0;
}