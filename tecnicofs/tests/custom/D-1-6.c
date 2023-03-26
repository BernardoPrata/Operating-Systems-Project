#include "fs/operations.h"
#include <assert.h>
#include <string.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE

int main() {

    assert(tfs_init() != -1);

    char buffer[MAX_SIZE];

    int file1 = tfs_open("/testfile", TFS_O_CREAT | TFS_O_TRUNC);
    assert(file1 != -1);    

    ssize_t r = tfs_read(file1, buffer, 10);
    assert(r == 0);

    printf("Successful test.\n");

    return 0;
}