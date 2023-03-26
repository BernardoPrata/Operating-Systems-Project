#ifndef COMMON_H
#define COMMON_H

#define FILENAME_SIZE 40

/* tfs_open flags */
enum {
    TFS_O_CREAT = 0b001,
    TFS_O_TRUNC = 0b010,
    TFS_O_APPEND = 0b100,
};

/* operation codes (for client-server requests) */
enum {
    TFS_OP_CODE_MOUNT = 1,
    TFS_OP_CODE_UNMOUNT = 2,
    TFS_OP_CODE_OPEN = 3,
    TFS_OP_CODE_CLOSE = 4,
    TFS_OP_CODE_WRITE = 5,
    TFS_OP_CODE_READ = 6,
    TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED = 7
};

/* operation message sizes (for client-server requests) */
#define TFS_MOUNT_SIZE (sizeof(char) * 41)
#define TFS_UNMOUNT_SIZE (sizeof(char) + sizeof(int))
#define TFS_OPEN_SIZE (sizeof(char) * 41 + sizeof(int) * 2)
#define TFS_CLOSE_SIZE (sizeof(char) + sizeof(int) * 2)
#define TFS_WRITE_SIZE_BEFORE_MESSAGE (sizeof(char) + sizeof(int) * 2 + sizeof(size_t))
#define TFS_READ_SIZE (sizeof(char) + sizeof(int) * 2 + sizeof(size_t))
#define TFS_SHUTDOWN_AFTER_ALL_CLOSED_SIZE (sizeof(char) + sizeof(int))

#endif /* COMMON_H */