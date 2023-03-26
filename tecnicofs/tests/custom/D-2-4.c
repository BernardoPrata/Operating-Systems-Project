#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


#define READ_SIZE 100
#define MAX_SIZE INODE_SIZE_AVAILABLE

int main() {

    assert(tfs_init() == 0);

    char src_file[] = "/testfile"; 
    char dest_file[] = "./tests/custom/.d_2_3.txt";

    int t = tfs_copy_to_external_fs(src_file, dest_file);
    assert(t != 0);

    printf("Successful test.\n");

    return 0;
}