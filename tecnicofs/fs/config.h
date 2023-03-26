#ifndef CONFIG_H
#define CONFIG_H

/* FS root inode number */
#define ROOT_DIR_INUM (0)

#define BLOCK_SIZE (1024)
#define DATA_BLOCKS (1024)
#define LEVEL0_BLOCK_NUM (10)
#define LEVEL1_BLOCK_NUM (1)
#define INODE_TABLE_SIZE (50)
#define MAX_OPEN_FILES (20)
#define MAX_FILE_NAME (40)
#define NO_FILES (0)
#define DELAY (5000)

#define TFS_ENABLE 0
#define TFS_DISABLE 1

#endif // CONFIG_H
