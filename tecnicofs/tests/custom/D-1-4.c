#include "fs/operations.h"
#include <assert.h>
#include <string.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define WRITING_SIZE 30000

int main() {

    assert(tfs_init() != -1);

    char bufferIn[MAX_SIZE];
    char bufferOut[MAX_SIZE];

    memset(bufferIn, 'A', sizeof(bufferIn));
    memset(bufferOut, 'B', sizeof(bufferIn));

    int file1 = tfs_open("/testfile", TFS_O_CREAT);
    assert(file1 != -1);    

    ssize_t r = tfs_write(file1, bufferIn, WRITING_SIZE);
    assert(r == WRITING_SIZE);

    int file2 = tfs_open("/testfile", 0);
    assert(file2 != -1);
    
    r = tfs_read(file2, bufferOut, MAX_SIZE);
    assert(r == WRITING_SIZE);

    assert(strncmp(bufferIn, bufferOut, WRITING_SIZE) == 0);
    assert(strncmp(bufferIn, bufferOut, WRITING_SIZE + 1) != 0);

    assert(tfs_close(file1) == 0);
    assert(tfs_close(file2) == 0);

    printf("Successful test.\n");

    return 0;
}
