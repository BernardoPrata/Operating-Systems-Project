#include "client/tecnicofs_client_api.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CLIENT_PIPE_NAME 40
#define S 4

/*  This test is similar to test1.c from the 1st exercise.
    The main difference is that this one explores the
    client-server architecture of the 2nd exercise.
    Explores how the server handles multiple requests from different clients to
   the same file
*/

void test(char *server_pipe, int client_id);
void shutdown_call(char *server_pipe, int client_id);

int child = -1;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf(
            "You must provide the following arguments: 'server_pipe_path'\n");
        return 1;
    }

    int pids[S];

    for (int i = 1; i < S; ++i) {
        child++;
        int pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            /* run test on child */
            test(argv[1], i);
            exit(0);

        } else {
            pids[i] = pid;
        }
    }
    /*
    Verifies call to  tfs_shutdown_after_all_closed,to
    confirm that the server does not terminate abruptly (at a time when
    no client has any files open).
    */

    shutdown_call(argv[1], 0);
    int result;

    for (int i = 1; i < S; ++i) {
        waitpid(pids[i], &result, 0);
        if (!WIFEXITED(result) || WEXITSTATUS(result) != 0) {
            printf("[Main client]: Process with pid %d failed\n", i);
            exit(1);
        } else
            printf("[Main client]: Client %d exited successfully.\n", i);
    }

    printf("[Test]: Calling shutdown_call\n");

    printf("Successful test.\n");

    return 0;
}

// similar to client_server_simple_test
void test(char *server_pipe, int client_id) {

    printf("[Client @%d]: Starting to run\n", client_id);
    char *path = "/f1";

    int f;

    char client_pipe[40];
    sprintf(client_pipe, "client%d.pipe", client_id);
    assert(tfs_mount(client_pipe, server_pipe) == 0);

    f = tfs_open(path, TFS_O_CREAT);
    tfs_close(f);

    printf("[Client @%d] End of child\n", client_id);
}

void shutdown_call(char *server_pipe, int client_id) {
    char client_pipe[40];
    sprintf(client_pipe, "clientPipe%d", client_id);
    assert(tfs_mount(client_pipe, server_pipe) == 0);
    assert(tfs_shutdown_after_all_closed() == 0);
}
