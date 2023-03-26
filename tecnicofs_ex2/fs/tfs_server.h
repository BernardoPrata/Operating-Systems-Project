#ifndef TFS_SERVER
#define TFS_SERVER

#include <stdbool.h>

#define MAX_SESSIONS 1000

typedef struct session {
    pthread_t tid;
    int id;
    int pipe;
    char* buffer;
    int counter;
    int prod_ptr; // Producer pointer
    int cons_ptr; // Consumer pointer
    bool has_message;
    pthread_cond_t message_placed;
    pthread_cond_t message_removed;
    bool running;
}* session_t;

int init_mutexes();
int destroy_mutexes();
int mount(char* pipe_name);
int unmount(int session_id);
session_t session_setup(int pipe, int id);
int session_destroy(session_t session);
void* session_run(void* arg);
int read_buffer(session_t session, char* dest);
int write_buffer(session_t session, char* src);
ssize_t read_pipe(int pipe, char* buffer);

#endif