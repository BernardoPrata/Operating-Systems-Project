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

    int file = tfs_open(src_file, TFS_O_CREAT);
    assert(file != -1);

    int t = tfs_copy_to_external_fs(src_file, dest_file);
    assert(t == 0);

    FILE* file_real = fopen(dest_file, "r");
    assert(file_real != NULL);

    char buffer_2[READ_SIZE + 70];
    assert(fread(buffer_2, sizeof(char), READ_SIZE, file_real) == 0);

    printf("Successful test.\n");

    return 0;
}