#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>    
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "operations.h"
#include "common/common.h"
#include "tfs_server.h"
    
#define BUFFER_SIZE 1000
#define PIPE_PATH_SIZE 40

int session_free_table[MAX_SESSIONS];
session_t sessions[MAX_SESSIONS];
pthread_mutex_t session_mutexes[MAX_SESSIONS];
pthread_mutex_t free_table_mutex = PTHREAD_MUTEX_INITIALIZER;
int session_count;
bool server_running = true;
pthread_mutex_t mutex_running = PTHREAD_MUTEX_INITIALIZER;
char* server_pipename;

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Please specify the pathname of the server's pipe.\n");
        return 1;
    }

    for (int i = 0; i < MAX_SESSIONS; i++) {
        session_free_table[i] = FREE;
    }

    server_pipename = argv[1];
    fprintf(stderr, "Starting TecnicoFS server with pipe called %s\n", server_pipename);

    if (init_mutexes() != 0) {
        fprintf(stderr, "[Server]: Mutexes initialization failed\n");
        return -1;
    }

    /**
     * Set up filesystem
     */

    if (tfs_init() != 0) {
        // Set errno
        fprintf(stderr, "[Server]: tfs_init failed\n");
        return -1;
    }


    /**
     * Set up pipe
     */
    int input_pipe;

    if (unlink(server_pipename) != 0 && errno != ENOENT ) {
        fprintf(stderr, "[Server]: Unlink of server pipe failed\n");
        return 1;
    }

    if (mkfifo(server_pipename, 0777) != 0) {
        fprintf(stderr, "[Server]: Mkfifo of server pipe failed\n");
        return 1;
    }

    char buffer[BUFFER_SIZE];

    input_pipe = open(server_pipename, O_RDONLY);
    if (input_pipe == -1) {
        fprintf(stderr, "[Server]: Server pipe open failed\n");
        return 1;
    }

    fprintf(stderr, "[Server]: Starting to receive messages\n");

    if (pthread_mutex_lock(&mutex_running) != 0) {
        return -1;
    }

    while (server_running) {

        if (pthread_mutex_unlock(&mutex_running) != 0) {
            return -1;
        }

        ssize_t ret = read_pipe(input_pipe, buffer);

        // Set up pipe again if running
        if (ret == 1) {
            if (pthread_mutex_lock(&mutex_running) != 0) {
                return -1;
            }

            if (server_running) {
                fprintf(stderr, "[Server]: Server pipe down. Reopening\n");
                input_pipe = open(server_pipename, O_RDONLY);
                if (input_pipe == -1) {
                    fprintf(stderr, "[Server]: Reopen of server pipe failed\n");
                    return 1;
                }
            }

            if (pthread_mutex_unlock(&mutex_running) != 0) {
                return -1;
            }

            continue;
        }

        if (buffer[0] == TFS_OP_CODE_MOUNT) {
            fprintf(stderr, "[Server]: New mount message received\n");
            buffer[PIPE_PATH_SIZE + 1] = 0;
            if (mount(buffer+1) == -1) {
                fprintf(stderr, "[Server]: Mount of client pipe failed\n");
                return -1;
            };
        }
        else {
            int session_id = (int) *((int*) (buffer + sizeof(char)));
            fprintf(stderr, "[Server]: New message received from session %d. Op code is %d\n", session_id, (int) buffer[0]);

            write_buffer(sessions[session_id], buffer);
        }

        if (pthread_mutex_lock(&mutex_running) != 0) {
            return -1;
        }
    }

    if (pthread_mutex_unlock(&mutex_running) != 0) {
        return -1;
    }

    destroy_mutexes();
    pthread_mutex_destroy(&free_table_mutex);
    pthread_mutex_destroy(&mutex_running);

    fprintf(stderr, "[Server]: Server terminated\n");
    return 0;
}

/**
 * Initializes all mutexes.
 * Returns 0 if successful and -1 otherwise.
 */
int init_mutexes() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (pthread_mutex_init(&session_mutexes[i], NULL) != 0) {
            return -1;
        }
    }

    return 0;
}

int destroy_mutexes() {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if(pthread_mutex_destroy(&session_mutexes[i]) != 0) {
            return -1;
        }
    }

    return 0;
}

/**
 * Reads message from pipe.
 * Returns 1 if pipe was closed, 0 if succesful and -1 otherwise.
 */
ssize_t read_pipe(int pipe, char* buffer) {
    ssize_t ret = read(pipe, buffer, sizeof(char)); 

    if (ret == 0) {
        return 1;
    }

    if (ret < sizeof(char)) {
        return -1;
    }

    int op_code = (int) *(buffer);
    buffer += sizeof(char);

    // Read remaining message (based on operation)

    switch (op_code) {
        case TFS_OP_CODE_MOUNT: {
            ret = read(pipe, buffer, TFS_MOUNT_SIZE - sizeof(char));
            if (ret <  TFS_MOUNT_SIZE - sizeof(char)) {
                return -1;
            }
            break;
        }
        case TFS_OP_CODE_UNMOUNT: {
            ret = read(pipe, buffer, TFS_UNMOUNT_SIZE - sizeof(char));
            if (ret <  TFS_UNMOUNT_SIZE - sizeof(char)) {
                return -1;
            }
            break;
        }
        case TFS_OP_CODE_OPEN: {
            ret = read(pipe, buffer, TFS_OPEN_SIZE - sizeof(char));
            if (ret <  TFS_OPEN_SIZE - sizeof(char)) {
                return -1;
            }
            break;
        }
        case TFS_OP_CODE_CLOSE: {
            ret = read(pipe, buffer, TFS_CLOSE_SIZE - sizeof(char));
            if (ret <  TFS_CLOSE_SIZE - sizeof(char)) {
                return -1;
            }
            break;
        }
        case TFS_OP_CODE_WRITE: {
            ret = read(pipe, buffer, TFS_WRITE_SIZE_BEFORE_MESSAGE - sizeof(char));

            if (ret < TFS_WRITE_SIZE_BEFORE_MESSAGE - sizeof(char)) {
                return -1;
            }
            
            size_t len = *((size_t*) (buffer + sizeof(int) * 2));
            ret = read(pipe, buffer + sizeof(int) * 2 + sizeof(size_t), len);

            if (ret < len) {
                return -1;
            }
            
            break;
        }
        case TFS_OP_CODE_READ: {
            ret = read(pipe, buffer, TFS_READ_SIZE - sizeof(char));
            if (ret <  TFS_READ_SIZE - sizeof(char)) {
                return -1;
            }
            break;
        }
        case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED: {
            ret = read(pipe, buffer, TFS_SHUTDOWN_AFTER_ALL_CLOSED_SIZE - sizeof(char));
            if (ret <  TFS_SHUTDOWN_AFTER_ALL_CLOSED_SIZE - sizeof(char)) {
                return -1;
            }
            break;
        }

        default: 
            return -1;
        

    }

    return 0;

}

/**
 * Mounts a new session using the argument as pathname for communication.
 * Returns the session id if successful. Returns -1 if unsuccessful.
 */
int mount(char* pipe_name) {
    int pipe = open(pipe_name, O_WRONLY);
    if (pipe == -1) {
        return -1;
    }

    int session_id = -1;

    // TODO: Mabye place this inside function
    if (pthread_mutex_lock(&free_table_mutex) != 0) {
        return -1;
    }

    for(session_id = 0; session_id < MAX_SESSIONS; session_id++) {
        if (session_free_table[session_id] == FREE) {
            session_free_table[session_id] = TAKEN;
            break;
        } 
    }

    // No sessions available
    if (session_id == -1) { return -1; }

    session_t session = session_setup(pipe, session_id);
    sessions[session_id] = session;

    if (pthread_mutex_unlock(&free_table_mutex) != 0) {
        return -1;
    }

    // Send the id to the client
    fprintf(stderr, "[Server]: Mount return: %d\n", session_id);

    ssize_t ret = write(pipe, (void*) &session_id, sizeof(int));
    if (ret != sizeof(int)) {
        return -1;
    }
    
    return 0;
}

/**
 * Sets up a new session.
 * Returns: the session if succesful. NULL if failed.
*/
session_t session_setup(int pipe, int id) {

    session_t session = (session_t) malloc(sizeof(struct session)); 
    if (session == NULL) {
        return NULL;
    }

    session->buffer = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    if (session->buffer == NULL) {
        free(session);
        return NULL;
    }

    session->pipe = pipe;
    session->id = id;
    session->counter = 0;
    session->prod_ptr = 0;
    session->cons_ptr = 0;
    session->has_message = false;
    session->running = true;

    if (pthread_cond_init(&session->message_placed, NULL) != 0 ||
        pthread_cond_init(&session->message_removed, NULL) != 0) {
        free(session->buffer);
        free(session);
        return NULL;
    }

    fprintf(stderr, "[Server]: Starting new session. Id: %d\n", session->id);
    if (pthread_create(&session->tid, NULL, (void*) session_run, session) != 0) {
        free(session->buffer);
        pthread_cond_destroy(&session->message_placed);
        pthread_cond_destroy(&session->message_removed);
        free(session);
        return NULL;
    }

    return session;
}

/**
 * Destroys a session and frees all memory needed. Does not close pipe.
 * Pipe fh must be saved before running session_destroy
 * Returns 0 if successful and -1 otherwise.
 */
int session_destroy(session_t session) {
    
    if (pthread_cond_destroy(&session->message_placed) != 0 ||  pthread_cond_destroy(&session->message_removed) != 0) {
        return -1;
    }

    free(session->buffer);
    free(session);

    return 0;
}

int session_unmount(session_t session, int current_id, bool warn_client) {
    fprintf(stderr, "[Server @%d]: Unmount requested\n", current_id);
    if (pthread_mutex_lock(&free_table_mutex) != 0) {
        return -1;
    }

    if (pthread_mutex_lock(&session_mutexes[current_id]) != 0) {
        return -1;
    }

    int pipe = session->pipe;

    session_free_table[current_id] = FREE;

    int ret = 0;
    if (session_destroy(session) != 0) {
        ret = -1;
    }

    pthread_mutex_unlock(&session_mutexes[current_id]);
    pthread_mutex_unlock(&free_table_mutex);

    fprintf(stderr, "[Server @%d]: Unmount return: %d\n", current_id, ret);
    if (warn_client && write(pipe, &ret, sizeof(int)) != sizeof(int)) {
        return -1;
    }

    close(pipe);

    return 0;
}

int session_open(session_t session, int current_id, char* buffer) {
    fprintf(stderr, "[Server @%d]: Open requested\n", current_id);
    char filename[FILENAME_SIZE + 1];
    memcpy(filename, buffer + sizeof(char) + sizeof(int), FILENAME_SIZE);
    filename[FILENAME_SIZE] = 0;

    int flags = *((int *)(buffer + sizeof(char) + sizeof(int) +
                          sizeof(char) * FILENAME_SIZE));

    fprintf(stderr, "[Server @%d]: Open called with (%s,%d)\n", current_id,
            filename, flags);

    int ret = tfs_open(filename, flags);

    fprintf(stderr, "[Server @%d]: Open return: %d\n", current_id, ret);
    if (write(session->pipe, (void *)&ret, sizeof(int)) != sizeof(int)) {
        return -1;
    }

    return 0;
}

int session_close(session_t session, int current_id, char* buffer) {
    fprintf(stderr, "[Server @%d]: Close requested\n", current_id);
    int fhandle = *((int *)(buffer + sizeof(char) + sizeof(int)));

    int ret = tfs_close(fhandle);

    fprintf(stderr, "[Server @%d]: Close return: %d\n", current_id, ret);
    if (write(session->pipe, (void *)&ret, sizeof(int)) != sizeof(int)) {
        return -1;
    }

    return 0;
}

int session_write(session_t session, int current_id, char* buffer) {
    fprintf(stderr, "[Server @%d]: Write requested\n", current_id);
    int fhandle = *((int *)(buffer + sizeof(char) + sizeof(int)));
    size_t len = *((size_t *)(buffer + sizeof(char) + sizeof(int) * 2));
    void *in_buffer = malloc(sizeof(char) * len);
    memcpy(in_buffer, buffer + sizeof(char) + sizeof(int) * 2 + sizeof(size_t),
           len);

    fprintf(stderr, "[Server @%d]: Write called with (%d, %s, %ld)\n",
            current_id, fhandle, (char *)in_buffer, len);
    ssize_t ret = tfs_write(fhandle, in_buffer, len);
    fprintf(stderr, "[Server @%d]: Write return: %ld\n", current_id, ret);
    if (write(session->pipe, (void *)&ret, sizeof(ssize_t)) !=
        sizeof(ssize_t)) {
        return -1;
    }

    return 0;
}

int session_read(session_t session, int current_id, char* buffer) {
    fprintf(stderr, "[Server @%d]: Read requested\n", current_id);
    int fhandle = *((int *)(buffer + sizeof(char) + sizeof(int)));
    size_t len = *((size_t *)(buffer + sizeof(char) + sizeof(int) * 2));
    void *ret_buffer = malloc(sizeof(char) * len);

    ssize_t ret = tfs_read(fhandle, ret_buffer, len);
    fprintf(stderr, "[Server @%d]: Read return: %ld %s\n", current_id, ret,
            (char *)ret_buffer);

    if (write(session->pipe, (void *)&ret, sizeof(ssize_t)) != sizeof(ssize_t)) {
        return -1;
    }

    if (ret != -1) {
        if (write(session->pipe, (void *)ret_buffer,
                  sizeof(char) * (size_t)ret) != sizeof(char) * (size_t)ret) {
            return -1;
        }
    }

    return 0;
}

int session_shutdown_all_after_closed(session_t session, int current_id) {
    fprintf(stderr, "[Server @%d]: Shutdown requested\n", current_id);

    int ret = tfs_destroy_after_all_closed();

    fprintf(stderr, "[Server @%d]: Destruction completed. Ending remaing threads\n", current_id);

    if (pthread_mutex_lock(&free_table_mutex) != 0) {
        return -1;
    }

    for (int other_session = 0; other_session < MAX_SESSIONS; other_session++) {
        if (other_session != current_id && session_free_table[other_session] == TAKEN) {
            if (pthread_mutex_lock(&session_mutexes[other_session]) != 0) {
                return -1;
            }

            sessions[other_session]->running = false;

            fprintf(stderr, "[Server @%d]: Thread %d to be killed.\n", current_id, other_session);

            if (pthread_cond_signal(&(sessions[other_session]->message_placed)) != 0) {
                return -1;
            }

            if (pthread_mutex_unlock(&session_mutexes[other_session]) != 0) {
                return -1;
            }
        }
    }

    if (pthread_mutex_unlock(&free_table_mutex) != 0) {
        return -1;
    }

    fprintf(stderr, "[Server @%d]: All other threads signaled to suicide\n", current_id);

    for (int other_session = 0; other_session < MAX_SESSIONS; other_session++) {
        if (other_session != current_id && session_free_table[other_session] == TAKEN) {
            if (pthread_join(sessions[other_session]->tid, NULL) != 0) {
                return -1;
            }

            fprintf(stderr, "[Server @%d]: Thread %d terminated. \n", current_id, other_session);
        }
    }

    fprintf(stderr, "[Server @%d]: All other threads terminated\n", current_id);


    fprintf(stderr, "[Server @%d]: Shutdown return: %d\n", current_id, ret);

    if (write(session->pipe, (void *)&ret, sizeof(int)) != sizeof(int)) {
        return -1;
    }

    // TODO: Check returns
    if (pthread_mutex_lock(&mutex_running) != 0) {
        return -1;
    }

    fprintf(stderr, "[Server @%d]: Main thread marked to end\n", current_id);

    server_running = false;
    if (pthread_mutex_unlock(&mutex_running) != 0) {
        return -1;
    }

    // Force main out of reading pipe
    // TODO: Check returns
    int temp_pipe = open(server_pipename, O_RDONLY);
    close(temp_pipe);

    return 0;
}

/**
 * Runs the sessoin passed as argument.
 */
void *session_run(void *arg) {
    session_t session = (session_t) arg;
    int current_id = session->id;
    char buffer[BUFFER_SIZE];

    while (true) {
        int ret = read_buffer(session, buffer);

        // TODO: maybe execute command first and then check this (the command is going to return -1 anyway)
        // Session is to be ended
        if (ret == -1 || ret == -2) {
            session_unmount(session, current_id, false);    
            return NULL;
        }


        switch(buffer[0]) {
            case TFS_OP_CODE_UNMOUNT:
                session_unmount(session, current_id, true);    
                return NULL;

            case TFS_OP_CODE_OPEN:
                if (session_open(session, current_id, buffer) != 0) {
                    session_unmount(session, current_id, false);
                    return NULL;
                }                
                break;

            case TFS_OP_CODE_CLOSE:
                if (session_close(session, current_id, buffer) != 0) {
                    session_unmount(session, current_id, false);
                    return NULL;
                }                
                break;

            case TFS_OP_CODE_WRITE:
                if (session_write(session, current_id, buffer) != 0) {
                    session_unmount(session, current_id, false);
                    return NULL;
                }                
                break;

            case TFS_OP_CODE_READ:
                if (session_read(session, current_id, buffer) != 0) {
                    session_unmount(session, current_id, false);
                    return NULL;
                }                
               break;
                
            case TFS_OP_CODE_SHUTDOWN_AFTER_ALL_CLOSED: 
                session_shutdown_all_after_closed(session, current_id);
                session_unmount(session, current_id, false);
                return NULL;

                break;

            default: 
                fprintf(stderr, "[Server @%d]: Unknown op_code\n", current_id);
                session_unmount(session, current_id, false);
                return NULL;
        }
    }
    return NULL;
}

/**
 * Synchronously reads from session buffer to dest
 * Returns the numbers of bytes read. Reads all the info in the buffer. Returns -2 if session was forced to end
 */
int read_buffer(session_t session, char* dest) {

    if (pthread_mutex_lock(&session_mutexes[session->id]) != 0) {
        return -1;
    }

    while (!session->has_message && session->running) {
        if (pthread_cond_wait(&session->message_placed, &session_mutexes[session->id]) != 0) {
            return -1;
        }
    }

    if (!session->running) {
        fprintf(stderr, "[Server @%d] Someone called shutdown. Unmounting\n", session->id);
        pthread_mutex_unlock(&session_mutexes[session->id]);
        return -2;
    }

    int counter = 0;

    // Read all possible
    while (session->counter > 0) {
        if (session->cons_ptr == BUFFER_SIZE) {
            session->cons_ptr = 0;
        }

        dest[counter++] = session->buffer[session->cons_ptr++];
        session->counter--;
    }

    session->has_message = false;

    if (pthread_cond_signal(&session->message_removed) != 0 ||
        pthread_mutex_unlock(&session_mutexes[session->id]) != 0) {
        return -1;
    }

    return counter;
}

/**
 * Synchronously writes from src into session buffer.
 * Returns the numbers of bytes written.
 */
int write_buffer(session_t session, char* src) {

    if (pthread_mutex_lock(&session_mutexes[session->id]) != 0) {
        return -1;
    }
    while (session->has_message) {
        if (pthread_cond_wait(&session->message_removed, &session_mutexes[session->id]) != 0) {
            return -1;
        }
    }
    
    int counter = 0;

    // Read all possible
    while (session->counter < BUFFER_SIZE) {
        if (session->prod_ptr == BUFFER_SIZE) {
            session->prod_ptr = 0;
        }

        session->buffer[session->prod_ptr++] = src[counter++];
        session->counter++;
    }

    session->has_message = true;

    if (pthread_cond_signal(&session->message_placed) != 0 || 
        pthread_mutex_unlock(&session_mutexes[session->id]) != 0) {
        return -1;
    }

    return counter;
}