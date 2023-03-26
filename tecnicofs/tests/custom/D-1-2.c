#include "fs/operations.h"
#include <assert.h>
#include <string.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define EXTRA 1000

int main() {

    assert(tfs_init() != -1);

    char bufferIn[MAX_SIZE + EXTRA];
    char bufferOut[MAX_SIZE];

    memset(bufferIn, 'A', sizeof(bufferIn));

    int fileIn = tfs_open("/testfile", TFS_O_CREAT);
    assert(fileIn != -1);    

    ssize_t r = tfs_write(fileIn, bufferIn, (MAX_SIZE + EXTRA) * sizeof(char));
    assert(r == MAX_SIZE);

    int fileOut = tfs_open("/testfile", 0);
    assert(fileOut != -1);
    
    r = tfs_read(fileOut, bufferOut, MAX_SIZE + EXTRA);
    assert(r == MAX_SIZE);
    assert(memcmp(bufferIn, bufferOut, MAX_SIZE) == 0);


    assert(tfs_close(fileIn) == 0);
    assert(tfs_close(fileOut) == 0);

    printf("Successful test.\n");

    return 0;
}
