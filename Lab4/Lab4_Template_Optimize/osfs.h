#ifndef _OSFS_H
#define _OSFS_H

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/bitmap.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/module.h>

#define OSFS_MAGIC 0x051AB520
// #define BLOCK_SIZE 4096
#define INODE_COUNT 20
#define DATA_BLOCK_COUNT 20
#define MAX_FILENAME_LEN 255
#define MAX_DIR_ENTRIES (BLOCK_SIZE / sizeof(struct osfs_dir_entry))

#define BITMAP_SIZE(bits) (((bits) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define INODE_BITMAP_SIZE BITMAP_SIZE(INODE_COUNT)
#define BLOCK_BITMAP_SIZE BITMAP_SIZE(DATA_BLOCK_COUNT)

#define ROOT_INODE 1

struct osfs_sb_info {
    uint32_t magic;
    uint32_t block_size;
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t nr_free_inodes;
    uint32_t nr_free_blocks;
    unsigned long *inode_bitmap;
    unsigned long *block_bitmap;
    void *inode_table;
    void *data_blocks;
};

struct osfs_dir_entry {
    char filename[MAX_FILENAME_LEN];
    uint32_t inode_no;
};

struct osfs_extent {
    uint32_t ee_start;    // 物理起始塊索引
    uint32_t ee_len;      // 連續區塊數量
};

struct osfs_inode {
    uint32_t i_ino;
    uint32_t i_size;
    uint32_t i_blocks;
    uint16_t i_mode;
    uint16_t i_links_count;
    uint32_t i_uid;
    uint32_t i_gid;
    struct timespec64 __i_atime;
    struct timespec64 __i_mtime;
    struct timespec64 __i_ctime;
    struct osfs_extent i_extent; // Extent-based
};

struct inode *osfs_iget(struct super_block *sb, unsigned long ino);
struct osfs_inode *osfs_get_osfs_inode(struct super_block *sb, uint32_t ino);
int osfs_get_free_inode(struct osfs_sb_info *sb_info);
int osfs_alloc_extent(struct osfs_sb_info *sb_info, uint32_t nr_blocks, uint32_t *start_block);
int osfs_fill_super(struct super_block *sb, void *data, int silent);
struct inode *osfs_new_inode(const struct inode *dir, umode_t mode);
void osfs_destroy_inode(struct inode *inode);

extern const struct inode_operations osfs_file_inode_operations;
extern const struct file_operations osfs_file_operations;
extern const struct inode_operations osfs_dir_inode_operations;
extern const struct file_operations osfs_dir_operations;
extern const struct super_operations osfs_super_ops;

#endif