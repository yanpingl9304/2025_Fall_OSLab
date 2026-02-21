#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "osfs.h"

int osfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct osfs_sb_info *sb_info;
    struct inode *root_inode;
    void *memory_region;
    
    // 計算所需總記憶體，確保每個區段邊界清晰
    size_t inode_bitmap_bytes = INODE_BITMAP_SIZE * sizeof(unsigned long);
    size_t block_bitmap_bytes = BLOCK_BITMAP_SIZE * sizeof(unsigned long);
    size_t inode_table_bytes = INODE_COUNT * sizeof(struct osfs_inode);
    size_t data_blocks_bytes = DATA_BLOCK_COUNT * BLOCK_SIZE;

    size_t total_size = sizeof(struct osfs_sb_info) + inode_bitmap_bytes + 
                        block_bitmap_bytes + inode_table_bytes + data_blocks_bytes;

    memory_region = vmalloc(total_size);
    if (!memory_region) return -ENOMEM;
    memset(memory_region, 0, total_size);

    sb_info = (struct osfs_sb_info *)memory_region;
    sb_info->magic = OSFS_MAGIC;
    sb_info->block_size = BLOCK_SIZE;
    sb_info->inode_count = INODE_COUNT;
    sb_info->block_count = DATA_BLOCK_COUNT;
    sb_info->nr_free_inodes = INODE_COUNT - 1;
    sb_info->nr_free_blocks = DATA_BLOCK_COUNT;

    // 分配位址
    sb_info->inode_bitmap = (unsigned long *)(sb_info + 1);
    sb_info->block_bitmap = sb_info->inode_bitmap + INODE_BITMAP_SIZE;
    sb_info->inode_table = (void *)(sb_info->block_bitmap + BLOCK_BITMAP_SIZE);
    sb_info->data_blocks = (void *)((char *)sb_info->inode_table + inode_table_bytes);

    sb->s_magic = OSFS_MAGIC;
    sb->s_fs_info = sb_info;
    sb->s_op = &osfs_super_ops;

    // 初始化 Root Inode
    root_inode = new_inode(sb);
    if (!root_inode) {
        vfree(memory_region);
        return -ENOMEM;
    }

    root_inode->i_ino = ROOT_INODE;
    root_inode->i_mode = S_IFDIR | 0755;
    root_inode->i_op = &osfs_dir_inode_operations;
    root_inode->i_fop = &osfs_dir_operations;
    set_nlink(root_inode, 2);
    
    struct osfs_inode *root_oi = osfs_get_osfs_inode(sb, ROOT_INODE);
    if (!root_oi) {
        iput(root_inode);
        vfree(memory_region);
        return -EIO;
    }
    
    root_oi->i_ino = ROOT_INODE;
    root_oi->i_mode = root_inode->i_mode;
    
    // 為目錄分配第一個 Extent 塊
    if (osfs_alloc_extent(sb_info, 1, &root_oi->i_extent.ee_start) == 0) {
        root_oi->i_extent.ee_len = 1;
        root_oi->i_blocks = 1;
    }

    root_inode->i_private = root_oi;
    set_bit(ROOT_INODE, sb_info->inode_bitmap);
    
    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        vfree(memory_region);
        return -ENOMEM;
    }
    return 0;
}

const struct super_operations osfs_super_ops = {
    .destroy_inode = osfs_destroy_inode,
    .statfs = simple_statfs,
};

void osfs_destroy_inode(struct inode *inode) {}