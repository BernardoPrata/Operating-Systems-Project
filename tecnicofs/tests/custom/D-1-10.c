#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

int main() {

    assert(tfs_init() != -1);

    // for (int i = 0; i < MAX_OPEN_FILES; i++) {
    //     assert(tfs_open("/test", TFS_O_CREAT) != -1);
    // }

    // assert(tfs_open("/test", TFS_O_CREAT) == -1);

    printf("Not ready.\n");

    return 0;
}