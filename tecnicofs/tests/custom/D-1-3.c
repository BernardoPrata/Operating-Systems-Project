#include "fs/operations.h"
#include <assert.h>
#include <string.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define READING_SIZE 3000

int main() {

    assert(tfs_init() != -1);

    char bufferIn[MAX_SIZE];
    char bufferIn2[MAX_SIZE];
    char bufferOut[MAX_SIZE];

    memset(bufferIn, 'A', sizeof(bufferIn));
    memset(bufferIn2, 'B', sizeof(bufferIn2));

    int file1 = tfs_open("/testfile", TFS_O_CREAT);
    assert(file1 != -1);    

    ssize_t r = tfs_write(file1, bufferIn, MAX_SIZE);
    assert(r == MAX_SIZE);

    int file2 = tfs_open("/testfile", 0);
    assert(file2 != -1);
    
    r = tfs_read(file2, bufferOut, READING_SIZE);
    assert(r == READING_SIZE);

    r = tfs_write(file2, bufferIn2, MAX_SIZE);
    assert(r == MAX_SIZE - READING_SIZE);

    int file3 = tfs_open("/testfile", 0);
    assert(file3 != -1);

    r = tfs_read(file3, bufferOut, MAX_SIZE);
    assert(r == MAX_SIZE);
    assert(bufferOut[READING_SIZE - 1] == 'A');
    assert(bufferOut[READING_SIZE] == 'B');

    assert(tfs_close(file1) == 0);
    assert(tfs_close(file2) == 0);

    printf("Successful test.\n");

    return 0;
}
