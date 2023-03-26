#include "fs/operations.h"
#include <assert.h>
#include <string.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE
#define ITERATIONS 100

int main() {

    assert(tfs_init() != -1);

    char buffer[MAX_SIZE];

    memset(buffer, 'Z', sizeof(buffer));
    int file1;

    for (int i = 0; i < DATA_BLOCKS - (INODE_SIZE_AVAILABLE / BLOCK_SIZE + LEVEL1_BLOCK_NUM + 1 /* the root one */); i++)
        data_block_alloc();

    // Only INODE_SIZE_AVAILABLE/BLOCK_SIZE + LEVEL1_BLOCK_NUM + 1 blocks will be available
    for (int i = 0; i < ITERATIONS; i++) {
        file1 = tfs_open("/testfile", TFS_O_CREAT | TFS_O_TRUNC);
        assert(file1 != -1);    

        ssize_t r = tfs_write(file1, buffer, MAX_SIZE);
        assert(r == MAX_SIZE);
        assert(tfs_close(file1) == 0);
    }

    printf("Successful test.\n");

    return 0;
}