#ifndef STATE_H
#define STATE_H

#include "config.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define INODE_BLOCK_NUM (LEVEL0_BLOCK_NUM + LEVEL1_BLOCK_NUM)
#define INDECES_PER_BLOCK (BLOCK_SIZE / sizeof(int))
#define INODE_SIZE_AVAILABLE                                                   \
    (BLOCK_SIZE * (LEVEL0_BLOCK_NUM + LEVEL1_BLOCK_NUM * INDECES_PER_BLOCK))
#define min(A, B) A < B ? A : B

#define mutex_lock(mutex) pthread_mutex_lock(&mutex)
#define mutex_unlock(mutex) pthread_mutex_unlock(&mutex)
#define read_lock(rwlock) pthread_rwlock_rdlock(&rwlock)
#define write_lock(rwlock) pthread_rwlock_wrlock(&rwlock)
#define rw_unlock(rwlock) pthread_rwlock_unlock(&rwlock)
#define init_mutex(mutex) pthread_mutex_init(&mutex, NULL)
#define init_rwlock(rwlock) pthread_rwlock_init(&rwlock, NULL)

typedef pthread_mutex_t mutex_t;
typedef pthread_rwlock_t rwlock_t;

/*
 * Directory entry
 */
typedef struct {
    char d_name[MAX_FILE_NAME];
    int d_inumber;
} dir_entry_t;

typedef enum { T_FILE, T_DIRECTORY } inode_type;

/*
 * I-node
 */
typedef struct {
    inode_type i_node_type;
    size_t i_size;
    int i_data_blocks[INODE_BLOCK_NUM];
    /* in a real FS, more fields would exist here */
} inode_t;

typedef enum { FREE = 0, TAKEN = 1 } allocation_state_t;

/*
 * Open file entry (in open file table)
 */
typedef struct {
    int of_inumber;
    size_t of_offset;
} open_file_entry_t;

#define MAX_DIR_ENTRIES (BLOCK_SIZE / sizeof(dir_entry_t))

void state_init();
void state_destroy();

int inode_create(inode_type n_type);
int inode_delete(int inumber);
inode_t *inode_get(int inumber);
int free_all_inode_blocks(int inumber);
int inode_data_block_get(size_t position, int inumber);
int inode_data_block_add(int inumber, int block_number, size_t position);

int clear_dir_entry(int inumber, int sub_inumber);
int add_dir_entry(int inumber, int sub_inumber, char const *sub_name);
int find_in_dir(int inumber, char const *sub_name);

int data_block_alloc();
int data_block_free(int block_number);
void *data_block_get(int block_number);

int add_to_open_file_table(int inumber, size_t offset);
int remove_from_open_file_table(int fhandle);
open_file_entry_t *get_open_file_entry(int fhandle);

// Lock functions
int inode_write_lock(int inumber);
int inode_read_lock(int inumber);
int inode_unlock(int inumber);
int free_inode_table_lock();
int free_inode_table_unlock();
int free_blocks_table_lock();
int free_blocks_table_unlock();
int open_file_lock(int fhandle);
int open_file_unlock(int fhandle);
int free_open_file_entries_table_lock();
int free_open_file_entries_table_unlock();
void lock_file_creation() ;
void unlock_file_creation();

#endif // STATE_H
