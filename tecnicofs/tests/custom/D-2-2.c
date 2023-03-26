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
    char dest_file[] = "./tests/custom/.d_2_2.txt";

    int file = tfs_open(src_file, TFS_O_CREAT);
    assert(file != -1);

    char buffer[READ_SIZE + 100];
    memset(buffer, 'a', READ_SIZE + 50);

    ssize_t r = tfs_write(file, buffer, READ_SIZE);
    assert(r == READ_SIZE);

    FILE* file_real_w = fopen(dest_file, "w");
    char existing_content[] = "some random content\n";
    assert(fwrite(existing_content, sizeof(char), strlen(existing_content), file_real_w) == strlen(existing_content));
    assert(fflush(file_real_w) != EOF && fclose(file_real_w) == 0);

    int t = tfs_copy_to_external_fs(src_file, dest_file);
    assert(t == 0);

    FILE* file_real = fopen(dest_file, "r");
    assert(file_real != NULL);

    char buffer_2[READ_SIZE + 70];
    assert(fread(buffer_2, sizeof(char), READ_SIZE, file_real) == READ_SIZE);

    assert(memcmp(buffer, buffer_2, sizeof(char) * READ_SIZE) == 0);

    printf("Successful test.\n");

    return 0;
}