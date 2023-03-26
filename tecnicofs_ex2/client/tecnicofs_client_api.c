#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>
#include "tecnicofs_client_api.h"
#include "common/common.h"

int session_id = -1;
int output;
int input;

// Returns -2 when new mount is needed
int tfs_mount(char const *client_pipe_path, char const *server_pipe_path) {

    if (unlink(client_pipe_path) != 0 && errno != ENOENT) {
        fprintf(stderr, "[Client]: Unlink failed: %s\n", strerror(errno));
        return -1;
    }

    if (mkfifo(client_pipe_path, 0777) != 0) {
        fprintf(stderr, "[Client]: Mkfifo failed: %s\n", strerror(errno));
        return -1;
    }

    fprintf(stderr, "[Client]: Mount requested. Client pipe: %s. Server pipe: %s\n", client_pipe_path, server_pipe_path);

    output = open(server_pipe_path, O_WRONLY);
    fprintf(stderr, "[Client]: Server pipe open terminated\n");

    if (output == -1) {
        return -1;
    }

    char msg[PIPE_SIZE + 1];
    msg[0] = TFS_OP_CODE_MOUNT;
    memset(msg + 1, 0, PIPE_SIZE);
    memcpy(msg + 1, client_pipe_path, strlen(client_pipe_path));

    fprintf(stderr, "[Client]: Mount message: %d %s\n", (int) *msg, msg + sizeof(char));

    if (write(output, msg, PIPE_SIZE + 1) != PIPE_SIZE + 1) {
        return -1;
    }

    fprintf(stderr, "[Client]: Mount message writen to pipe\n");

    input = open(client_pipe_path, O_RDONLY);
    if (input == -1) {
        return -1;
    }
    
    fprintf(stderr, "[Client]: Client pipe open terminated\n");

    int ret;
    ssize_t read_ret = read(input, (void*) &ret, sizeof(int));

    // Server closed pipe. Mount is needed
    if (read_ret == 0) {
        return -2;
    }

    if (read_ret != sizeof(int)) {
        return -1;
    }

    session_id = ret;

    fprintf(stderr, "[Client]: Mount terminated. Session id: %d\n", session_id);

    return 0;
}

int tfs_unmount() {

    if (session_id == -1) {
        return -1;
    }

    // Write
    size_t msg_size = sizeof(char) + sizeof(int);
    char* msg = (char*) malloc(msg_size);
    msg[0] = TFS_OP_CODE_UNMOUNT;
    memcpy(msg + sizeof(char), &session_id, sizeof(int));

    fprintf(stderr, "[Client @%d]: Unmount message: %d %d\n", session_id, (int) *msg, session_id);

    if (write(output, msg, msg_size) != msg_size) {
        return -1;
    }

    // Read
    int ret;
    ssize_t read_ret = read(input, (void*) &ret, sizeof(int));

    if (read_ret == 0) {
        return -2;
    }

    if (read_ret != sizeof(int)) {
        return -1;
    }

    return ret;
}

int tfs_open(char const *name, int flags) {

    if (session_id == -1) {
        return -1;
    }

    // Write
    size_t msg_size = sizeof(char) * (1 + FILENAME_SIZE) + sizeof(int) * 2;
    char* msg = (char*) malloc(msg_size);
    msg[0] = TFS_OP_CODE_OPEN;
    memcpy(msg + sizeof(char), &session_id, sizeof(int));
    memset(msg + sizeof(char) + sizeof(int), 0, FILENAME_SIZE);
    memcpy(msg + sizeof(char) + sizeof(int), name, strlen(name));
    memcpy(msg + sizeof(char) * (1 + FILENAME_SIZE) + sizeof(int), &flags, sizeof(int));

    fprintf(stderr, "[Client @%d]: Open for %s requested\n", session_id, name);
    fprintf(stderr, "[Client @%d]: Open message: %d %d %s\n", session_id,
        (int) *msg, *((int*) (msg + 1)), msg + sizeof(int) + sizeof(char));

    if (write(output, msg, msg_size) != msg_size) {
        return -1;
    }

    // Read
    int ret;
    ssize_t read_ret = read(input, (void*) &ret, sizeof(int));

    if (read_ret == 0) {
        return -2;
    }

    if (read_ret != sizeof(int)) {
        return -1;
    }

    fprintf(stderr, "[Client @%d]: Open terminated\n", session_id);

    return ret;
}

int tfs_close(int fhandle) {

    if (session_id == -1) {
        return -1;
    }

    // Write
    size_t msg_size = sizeof(char) + sizeof(int) * 2;
    char* msg = (char*) malloc(msg_size);
    msg[0] = TFS_OP_CODE_CLOSE;
    memcpy(msg + sizeof(char), &session_id, sizeof(int));
    memcpy(msg + sizeof(int) + sizeof(char), &fhandle, sizeof(int));

    fprintf(stderr, "[Client @%d]: Close message: %d %d %d\n", session_id,
        (int) *msg, (int) *((int*)(msg + sizeof(char))), (int) *((int*)(msg + sizeof(char) + sizeof(int))));

    if (write(output, msg, msg_size) != msg_size) {
        return -1;
    }

    // Read
    int ret;
    ssize_t read_ret = read(input, (void*) &ret, sizeof(int));

    if (read_ret == 0) {
        return -2;
    }

    if (read_ret != sizeof(int)) {
        return -1;
    }

    fprintf(stderr, "[Client @%d]: Close terminated\n", session_id);

    return ret;
}

ssize_t tfs_write(int fhandle, void const *buffer, size_t len) {

    if (session_id == -1) {
        return -1;
    }

    fprintf(stderr, "[Client @%d]: Write requested", session_id);

    // Write

    // Truncate if write might not be atomic
    if (TFS_WRITE_SIZE_BEFORE_MESSAGE + len > PIPE_BUF) {
        len = PIPE_BUF - TFS_WRITE_SIZE_BEFORE_MESSAGE;
    }
    size_t msg_size = sizeof(char) * (1 + len) + sizeof(int) * 2 + sizeof(size_t) ;
    char* msg = (char*) malloc(msg_size);
    msg[0] = TFS_OP_CODE_WRITE;
    memcpy(msg + sizeof(char), &session_id, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int), &fhandle, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int) * 2, &len, sizeof(size_t));
    memcpy(msg + sizeof(char) + sizeof(int) * 2 + sizeof(size_t), buffer, sizeof(char) * len);

    if (write(output, msg, msg_size) != msg_size) {
        return -1;
    }

    // Read
    ssize_t ret;
    ssize_t read_ret = read(input, (void*) &ret, sizeof(ssize_t));

    if (read_ret == 0) {
        return -2;
    }

    if (read_ret != sizeof(ssize_t)) {
        return -1;
    }

    fprintf(stderr, "[Client @%d]: Write terminated\n", session_id);

    return ret;
}

ssize_t tfs_read(int fhandle, void *buffer, size_t len) {

    if (session_id == -1) {
        return -1;
    }

    // Write
    size_t msg_size = sizeof(char) + sizeof(int) * 2 + sizeof(size_t);
    char* msg = (char*) malloc(msg_size);
    msg[0] = TFS_OP_CODE_READ;
    memcpy(msg + sizeof(char), &session_id, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int), &fhandle, sizeof(int));
    memcpy(msg + sizeof(char) + sizeof(int) * 2, &len, sizeof(size_t));

    fprintf(stderr, "[Client @%d]: Read message: %d %d %d %d\n", session_id,
        (int) *msg, (int) *((int*)(msg + sizeof(char))),(int) *((int*)(msg + sizeof(char) + sizeof(int))),
        (int) *((int*)(msg + sizeof(char) + sizeof(int) * 2)));

    if (write(output, msg, msg_size) != msg_size) {
        fprintf(stderr, "[Client @%d]: Read failed\n", session_id);
        return -1;
    }

    // Read
    ssize_t count;
    ssize_t read_ret = read(input, (void*) &count, sizeof(ssize_t));

    if (read_ret == 0) {
        return -2;
    }

    if (read_ret != sizeof(ssize_t)) {
        return -1;
    }

    // TODO: Maybe remove this intermediate buffer
    char* return_buffer = (char*) malloc(sizeof(char) * (unsigned) (count + 1));
    return_buffer[count] = 0;

    read_ret = read(input, (void*) return_buffer, sizeof(char) * (unsigned) count);
    
    if (read_ret == 0) {
        return -2;
    }

    if (read_ret != sizeof(char) * (unsigned) count) {
        return -1;
    }

    fprintf(stderr, "[Client @%d]: Read return: %ld %s\n", session_id, count, return_buffer);

    memcpy(buffer, (void*) return_buffer, (size_t) count);

    fprintf(stderr, "[Client @%d]: Read successful\n", session_id);

    return count;
}

int tfs_shutdown_after_all_closed() {

    if (session_id == -1) {
        return -1;
    }

    // Write
    size_t msg_size = sizeof(char) + sizeof(int);
    char* msg = (char*) malloc(msg_size);
    msg[0] = TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED;
    memcpy(msg + sizeof(char), &session_id, sizeof(int));

    fprintf(stderr, "[Client @%d]: Shutdown message: %d %d\n", session_id, (int) *msg, (int) *((int*) (msg + 1)));

    if (write(output, msg, msg_size) != msg_size) {
        return -1;
    }

    // Read
    int ret[1];
    ssize_t read_ret = read(input, (void*) ret, sizeof(int));

    if (read_ret == 0) {
        return -2;
    }
    
    if (read_ret != sizeof(int)) {
        return -1;
    }

    fprintf(stderr, "[Client @%d]: Shutdown terminated\n", session_id);

    session_id = -1;

    return ret[0];
}
