#include "fs/operations.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE INODE_SIZE_AVAILABLE

typedef struct {
    double pressure_wheels;
    int speed;
    char* name;
} car;


int main() {

    assert(tfs_init() != -1);

    int bufferInt[MAX_SIZE / sizeof(int)];
    float bufferFloat[MAX_SIZE / sizeof(float)];
    car bufferCar[MAX_SIZE / sizeof(car)];
    
     int bufferIntResult[MAX_SIZE / sizeof(int)];
    float bufferFloatResult[MAX_SIZE / sizeof(float)];
    car bufferCarResult[MAX_SIZE / sizeof(car)];

    for (int i = 0; i < MAX_SIZE / sizeof(int); i++) {
        bufferInt[i] = 3;
    }

    for (int i = 0; i < MAX_SIZE / sizeof(float); i++) {
        bufferFloat[i] = 4.5;
    }

    for (int i = 0; i < MAX_SIZE / sizeof(car); i++) {
        car c;
        c.pressure_wheels = 3.4;
        c.speed = 5;
        c.name = "carrinho";
        bufferCar[i] = c;
    }

    int file1 = tfs_open("/file_int", TFS_O_CREAT);
    assert(file1 != -1);
    int file2 = tfs_open("/file_float", TFS_O_CREAT);
    assert(file2 != -1);
    int file3 = tfs_open("/file_car", TFS_O_CREAT);
    assert(file3 != -1);

    ssize_t r;

    r = tfs_write(file1, bufferInt, sizeof(bufferInt));
    assert(r == sizeof(bufferInt));
    r = tfs_write(file2, bufferFloat, sizeof(bufferFloat));
    assert(r == sizeof(bufferFloat));
    r = tfs_write(file3, bufferCar, sizeof(bufferCar));
    assert(r == sizeof(bufferCar));

    int file4 = tfs_open("/file_int", 0);
    assert(file4 != -1);
    int file5 = tfs_open("/file_float", 0);
    assert(file5 != -1);
    int file6 = tfs_open("/file_car", 0);
    assert(file6 != -1);

    r = tfs_read(file4, bufferIntResult, sizeof(bufferInt));
    assert(r == sizeof(bufferInt));
    r = tfs_read(file5, bufferFloatResult, sizeof(bufferFloat));
    assert(r == sizeof(bufferFloat));
    r = tfs_read(file6, bufferCarResult, sizeof(bufferCar));
    assert(r == sizeof(bufferCar));

    assert(memcmp(bufferInt, bufferIntResult, sizeof(bufferInt)) == 0);
    assert(memcmp(bufferFloat, bufferFloatResult, sizeof(bufferFloat)) == 0);
    assert(memcmp(bufferCar, bufferCarResult, sizeof(bufferCar)) == 0);

    printf("Successful test.\n");

    return 0;
}