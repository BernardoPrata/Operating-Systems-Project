#include "state.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Persistent FS state  (in reality, it should be maintained in secondary
 * memory; for simplicity, this project maintains it in primary memory) */

/* I-node table */
static inode_t inode_table[INODE_TABLE_SIZE];
static char freeinode_ts[INODE_TABLE_SIZE];

/* Data blocks */
static char fs_data[BLOCK_SIZE * DATA_BLOCKS];
static char free_blocks[DATA_BLOCKS];

/* Volatile FS state */
static open_file_entry_t open_file_table[MAX_OPEN_FILES];
static char free_open_file_entries[MAX_OPEN_FILES];

/* Locks   */

static rwlock_t inodes_locks[INODE_TABLE_SIZE];
static mutex_t freeinode_lock;
static mutex_t free_blocks_lock;
static mutex_t open_files_locks[MAX_OPEN_FILES];
static mutex_t free_open_file_entries_lock;
static mutex_t create_file_lock;

static inline bool valid_inumber(int inumber) {
    return inumber >= 0 && inumber < INODE_TABLE_SIZE;
}

static inline bool valid_block_number(int block_number) {
    return block_number >= 0 && block_number < DATA_BLOCKS;
}

static inline bool valid_file_handle(int file_handle) {
    return file_handle >= 0 && file_handle < MAX_OPEN_FILES;
}

/**
 * We need to defeat the optimizer for the insert_delay() function.
 * Under optimization, the empty loop would be completely optimized away.
 * This function tells the compiler that the assembly code being run (which is
 * none) might potentially change *all memory in the process*.
 *
 * This prevents the optimizer from optimizing this code away, because it does
 * not know what it does and it may have side effects.
 *
 * Reference with more information: https://youtu.be/nXaxk27zwlk?t=2775
 *
 * Exercise: try removing this function and look at the assembly generated to
 * compare.
 */
static void touch_all_memory() { __asm volatile("" : : : "memory"); }

/*
 * Auxiliary function to insert a delay.
 * Used in accesses to persistent FS state as a way of emulating access
 * latencies as if such data structures were really stored in secondary memory.
 */
static void insert_delay() {
    for (int i = 0; i < DELAY; i++) {
        touch_all_memory();
    }
}

/*
 * Initializes FS state
 */
void state_init() {
    init_mutex(freeinode_lock);
    init_mutex(free_blocks_lock);
    init_mutex(free_open_file_entries_lock);
    init_mutex(create_file_lock);

    for (size_t i = 0; i < INODE_TABLE_SIZE; i++) {
        freeinode_ts[i] = FREE;
        init_rwlock(inodes_locks[i]);
    }

    for (size_t i = 0; i < DATA_BLOCKS; i++) {
        free_blocks[i] = FREE;
    }

    for (size_t i = 0; i < MAX_OPEN_FILES; i++) {
        free_open_file_entries[i] = FREE;
        init_mutex(open_files_locks[i]);
    }
}

void state_destroy() { /* nothing to do */
    // TODO: below must destroy every single structure lock
}

/*
 * Creates a new i-node in the i-node table.
 * Input:
 *  - n_type: the type of the node (file or directory)
 * Returns:
 *  new i-node's number if successfully created, -1 otherwise
 */
int inode_create(inode_type n_type) {
    free_inode_table_lock();
    for (int inumber = 0; inumber < INODE_TABLE_SIZE; inumber++) {
        if ((inumber * (int)sizeof(allocation_state_t) % BLOCK_SIZE) == 0) {
            insert_delay(); // simulate storage access delay (to freeinode_ts)
        }

        /* Finds first free entry in i-node table */
        if (freeinode_ts[inumber] == FREE) {
            /* Found a free entry, so takes it for the new i-node*/
            inode_write_lock(inumber);
            freeinode_ts[inumber] = TAKEN;
            insert_delay(); // simulate storage access delay (to i-node)
            inode_table[inumber].i_node_type = n_type;

            if (n_type == T_DIRECTORY) {
                /* Initializes directory (filling its block with empty
                 * entries, labeled with inumber==-1) */
                int b = data_block_alloc();
                if (b == -1) {
                    freeinode_ts[inumber] = FREE;
                    inode_unlock(inumber);
                    free_inode_table_unlock();
                    return -1;
                }

                inode_table[inumber].i_size = BLOCK_SIZE;
                inode_table[inumber].i_data_blocks[0] = b;

                dir_entry_t *dir_entry = (dir_entry_t *)data_block_get(b);
                if (dir_entry == NULL) {
                    freeinode_ts[inumber] = FREE;
                    inode_unlock(inumber);
                    free_inode_table_unlock();
                    return -1;
                }

                for (size_t i = 0; i < MAX_DIR_ENTRIES; i++) {
                    dir_entry[i].d_inumber = -1;
                }
            } else {
                int i;

                /* In case of a new file, simply sets its size to 0 */
                inode_table[inumber].i_size = 0;
                for (i = 0; i < INODE_BLOCK_NUM; i++) {
                    inode_table[inumber].i_data_blocks[i] = -1;
                }
            }

            inode_unlock(inumber);
            free_inode_table_unlock();
            return inumber;
        }
    }
    free_inode_table_unlock();

    return -1;
}

/*
 * Frees all the blocks used by inode.
 * Input:
 *  - inumber: i-node's number
 * Returns: 0 if successful, -1 if failed
 */
int free_all_inode_blocks(int inumber) {
    int i, block_number;

    // Delete direct references
    free_blocks_table_lock();
    for (i = 0; i < INODE_BLOCK_NUM - 1; i++) {
        block_number = inode_table[inumber].i_data_blocks[i];
        inode_table[inumber].i_data_blocks[i] = -1;
        if (block_number != -1 && data_block_free(block_number) == -1) {
            free_blocks_table_unlock();

            return -1;
        }
    }

    // Delete blocks on 1st level
    if (inode_table[inumber].i_data_blocks[i] != -1) {
        int *level1_block =
            (int *)data_block_get(inode_table[inumber].i_data_blocks[i]);

        if (level1_block == NULL) {
            free_blocks_table_unlock();

            return -1;
        }

        int j = 0;
        while (level1_block[j] != -1 && j < INDECES_PER_BLOCK) {
            if (data_block_free(level1_block[j]) == -1) {
                free_blocks_table_unlock();

                return -1;
            }
            j++;
        }

        // Delete the block of deference
        if (data_block_free(
                inode_table[inumber].i_data_blocks[INODE_BLOCK_NUM - 1]) ==
            -1) {
            free_blocks_table_unlock();

            return -1;
        }
        inode_table[inumber].i_data_blocks[INODE_BLOCK_NUM - 1] = -1;
    }
    free_blocks_table_unlock();

    return 0;
}

/*
 * Input:
 *  - position: position of data block in i-node (indexed at 0)
 *  - inumber: i-node's number
 * Returns: index of block if successful, -1 if failed
 */
int inode_data_block_get(size_t position, int inumber) {
    if (position >= INODE_SIZE_AVAILABLE / BLOCK_SIZE) {
        return -1;
    }

    inode_t *inode = inode_get(inumber);

    if (inode == NULL) {
        return -1;
    }

    if (position < LEVEL0_BLOCK_NUM)
        return inode->i_data_blocks[position];

    position -= LEVEL0_BLOCK_NUM;
    size_t level1_block_index = position / INDECES_PER_BLOCK + LEVEL0_BLOCK_NUM;
    size_t position_inside_block = position % INDECES_PER_BLOCK;

    if (inode->i_data_blocks[level1_block_index] == -1) {
        return -1;
    }

    int *level1_block =
        (int*) data_block_get(inode->i_data_blocks[level1_block_index]);

    return level1_block[position_inside_block];
}

/* TODO: Find a way to be an appending only (error if i place not at the end)
 Probably add count of blocks used to i-node and drop the position argument */

/*
 * Adds a block to a free position in the list of data blocks in i-node
 * Input:
 *  - inumber: i-node's number
 *  - block_number: block's number
 *  - position: position in which the block must be added
 * Returns: 0 if successful, -1 if failed
 */
int inode_data_block_add(int inumber, int block_number, size_t position) {
    inode_t *inode = inode_get(inumber);

    if (position > INODE_SIZE_AVAILABLE / BLOCK_SIZE) {
        return -1;
    }

    if (position < LEVEL0_BLOCK_NUM) {
        if (inode->i_data_blocks[position] != -1)
            return -1;
        inode->i_data_blocks[position] = block_number;
        return 0;
    }

    position -= LEVEL0_BLOCK_NUM;
    size_t level1_block_index = position / INDECES_PER_BLOCK + LEVEL0_BLOCK_NUM;
    size_t position_inside_block = position % INDECES_PER_BLOCK;

    int *level1_block;

    if (inode->i_data_blocks[level1_block_index] == -1) {
        inode->i_data_blocks[level1_block_index] = data_block_alloc();
        level1_block =
            (int *)data_block_get(inode->i_data_blocks[level1_block_index]);
        for (int i = 0; i < BLOCK_SIZE; i++)
            level1_block[i] = -1;
    } else
        level1_block =
            (int *)data_block_get(inode->i_data_blocks[level1_block_index]);

    if (level1_block[position_inside_block] != -1)
        return -1;
    level1_block[position_inside_block] = block_number;

    return 0;
}

/*
 * Deletes the i-node. Requires caller to have acquired inode's lock.
 * Input:
 *  - inumber: i-node's number
 * Returns: 0 if successful, -1 if failed
 */
int inode_delete(int inumber) {

    // simulate storage access delay (to i-node and freeinode_ts)
    insert_delay();
    insert_delay();

    if (!valid_inumber(inumber))
        return -1;

    free_inode_table_lock();
    inode_write_lock(inumber);

    if (freeinode_ts[inumber] == FREE) {
        free_inode_table_unlock();

        return -1;
    }

    inode_t *inode = inode_get(inumber);
    if (inode == NULL) {
        free_inode_table_unlock();
        return -1;
    }


    freeinode_ts[inumber] = FREE;

    if (inode->i_size > 0) {
        if (free_all_inode_blocks(inumber) != 0) {
            free_inode_table_unlock();
            return -1;
        }
    }

    inode_unlock(inumber);
    free_inode_table_unlock();

    return 0;
}

/*
 * Returns a pointer to an existing i-node.
 * Input:
 *  - inumber: identifier of the i-node
 * Returns: pointer if successful, NULL if failed
 */
inode_t *inode_get(int inumber) {
    if (!valid_inumber(inumber)) {
        return NULL;
    }

    insert_delay(); // simulate storage access delay to i-node
    return &inode_table[inumber];
}

/*
 * Adds an entry to the i-node directory data.
 * Input:
 *  - inumber: identifier of the i-node
 *  - sub_inumber: identifier of the sub i-node entry
 *  - sub_name: name of the sub i-node entry
 * Returns: SUCCESS or FAIL
 */
int add_dir_entry(int inumber, int sub_inumber, char const *sub_name) {
    // tem de ser chamada com i-node bloqueado-> chamada em tfs_open
    if (!valid_inumber(inumber) || !valid_inumber(sub_inumber)) {
        return -1;
    }

    if (strlen(sub_name) == 0) {
        return -1;
    }


    insert_delay(); // simulate storage access delay to i-node with inumber
    if (inode_table[inumber].i_node_type != T_DIRECTORY) {
        return -1;
    }

    /* Locates the block containing the directory's entries */
    dir_entry_t *dir_entry =
        (dir_entry_t *)data_block_get(inode_table[inumber].i_data_blocks[0]);

    if (dir_entry == NULL) {
        return -1;
    }


    /* Finds and fills the first empty entry */
    for (size_t i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir_entry[i].d_inumber == -1) {
            dir_entry[i].d_inumber = sub_inumber;
            strncpy(dir_entry[i].d_name, sub_name, MAX_FILE_NAME - 1);
            dir_entry[i].d_name[MAX_FILE_NAME - 1] = 0;
            return 0;
        }
    }

    return -1;
}

/* Looks for a given name inside a directory
 * Input:
 * 	- parent directory's i-node number
 * 	- name to search
 * 	Returns i-number linked to the target name, -1 if not found
 */
int find_in_dir(int inumber, char const *sub_name) {

    // chamada uma vez em tfs_lookup -> tenho de  bloquear i-node porque isto
    // acede aos blocos através de data_block_get

    insert_delay(); // simulate storage access delay to i-node with inumber
    inode_read_lock(inumber);
    if (!valid_inumber(inumber) ||
        inode_table[inumber].i_node_type != T_DIRECTORY) {
        inode_unlock(inumber);

        return -1;
    }

    /* Locates the block containing the directory's entries */
    dir_entry_t *dir_entry =
        (dir_entry_t *)data_block_get(inode_table[inumber].i_data_blocks[0]);
    if (dir_entry == NULL) {
        inode_unlock(inumber);
        return -1;
    }

    /* Iterates over the directory entries looking for one that has the target
     * name */
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if ((dir_entry[i].d_inumber != -1) &&
            (strncmp(dir_entry[i].d_name, sub_name, MAX_FILE_NAME) == 0)) {
            inode_unlock(inumber);
            return dir_entry[i].d_inumber;
        }
    }
    inode_unlock(inumber);
    return -1;
}

/*
 * Allocated a new data block
 * Returns: block index if successful, -1 otherwise
 */
int data_block_alloc() {
    free_blocks_table_lock();
    for (int i = 0; i < DATA_BLOCKS; i++) {
        if (i * (int)sizeof(allocation_state_t) % BLOCK_SIZE == 0) {
            insert_delay(); // simulate storage access delay to free_blocks
        }

        if (free_blocks[i] == FREE) {
            free_blocks[i] = TAKEN;
            free_blocks_table_unlock();

            return i;
        }
    }
    free_blocks_table_unlock();

    return -1;
}

/* Frees a data block
 * Input
 * 	- the block index
 * Returns: 0 if success, -1 otherwise
 */
int data_block_free(int block_number) {
    if (!valid_block_number(block_number)) {
        return -1;
    }
    // No need to Lock, cuz is only called when
    // //pthread_mutex_lock(&dataFreeMap); //

    insert_delay(); // simulate storage access delay to free_blocks

    free_blocks[block_number] = FREE;

    return 0;
}

/* Returns a pointer to the contents of a given block
 * Input:
 * 	- Block's index
 * Returns: pointer to the first byte of the block, NULL otherwise
 */
void *data_block_get(int block_number) {
    if (!valid_block_number(block_number)) {
        return NULL;
    }

    insert_delay(); // simulate storage access delay to block
    return &fs_data[block_number * BLOCK_SIZE];
}

/* Add new entry to the open file table
 * Inputs:
 * 	- I-node number of the file to open
 * 	- Initial offset
 * Returns: file handle if successful, -1 otherwise
 */
int add_to_open_file_table(int inumber, size_t offset) {
    free_open_file_entries_table_lock();
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (free_open_file_entries[i] == FREE) {
            free_open_file_entries[i] = TAKEN;
            open_file_lock(i);
            open_file_table[i].of_inumber = inumber;
            open_file_table[i].of_offset = offset;
            open_file_unlock(i);
            free_open_file_entries_table_unlock();
            return i;
        }
    }
    free_open_file_entries_table_unlock();
    return -1;
}

/* Frees an entry from the open file table. Open file lock must be locked
 * Inputs:
 * 	- file handle to free/close
 * Returns 0 is success, -1 otherwise
 */
int remove_from_open_file_table(int fhandle) {

    if (!valid_file_handle(fhandle)) {
        return -1;
    }

    free_open_file_entries_table_lock();
    open_file_lock(fhandle);

    if (free_open_file_entries[fhandle] != TAKEN) {
        free_open_file_entries_table_unlock();
        return -1;
    }

    free_open_file_entries[fhandle] = FREE;

    open_file_unlock(fhandle);
    free_open_file_entries_table_unlock();

    return 0;
}

/* Returns pointer to a given entry in the open file table
 * Inputs:
 * 	 - file handle
 * Returns: pointer to the entry if sucessful, NULL otherwise
 */
open_file_entry_t *get_open_file_entry(int fhandle) {
    if (!valid_file_handle(fhandle)) {
        return NULL;
    }
    return &open_file_table[fhandle];
}


int inode_write_lock(int inumber) {
    if (!valid_inumber(inumber)) {
        return -1;
    }

    return write_lock(inodes_locks[inumber]);
}

int inode_read_lock(int inumber) {
    if (!valid_inumber(inumber)) {
        return -1;
    }

    return read_lock(inodes_locks[inumber]);
}

int inode_unlock(int inumber) {
    if (!valid_inumber(inumber)) {
        return -1;
    }

    return rw_unlock(inodes_locks[inumber]);
}

int free_inode_table_lock() {
    return mutex_lock(freeinode_lock); 
}

int free_inode_table_unlock() {
    return mutex_unlock(freeinode_lock);
}

int free_blocks_table_lock() {
    return mutex_lock(free_blocks_lock);
}

int free_blocks_table_unlock() {
    return mutex_unlock(free_blocks_lock);
}

int open_file_lock(int fhandle) {
    if (!valid_file_handle(fhandle)) {
        return -1;
    }

    return mutex_lock(open_files_locks[fhandle]);
}

int open_file_unlock(int fhandle) {
    if (!valid_file_handle(fhandle)) {
        return -1;
    }
    
    return mutex_unlock(open_files_locks[fhandle]);
}

int free_open_file_entries_table_lock() {
    return mutex_lock(free_open_file_entries_lock);
}

int free_open_file_entries_table_unlock() {
    return mutex_unlock(free_open_file_entries_lock);
}

void lock_file_creation() {
    mutex_lock(create_file_lock);
}

void unlock_file_creation() {
    mutex_unlock(create_file_lock);
}

