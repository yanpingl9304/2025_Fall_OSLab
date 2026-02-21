#include <linux/fs.h>
#include "osfs.h"

struct osfs_inode *osfs_get_osfs_inode(struct super_block *sb, uint32_t ino) {
    struct osfs_sb_info *sb_info = sb->s_fs_info;
    if (ino == 0 || ino >= sb_info->inode_count) return NULL;
    return &((struct osfs_inode *)(sb_info->inode_table))[ino];
}

int osfs_get_free_inode(struct osfs_sb_info *sb_info) {
    uint32_t ino;
    for (ino = 1; ino < sb_info->inode_count; ino++) {
        if (!test_bit(ino, sb_info->inode_bitmap)) {
            set_bit(ino, sb_info->inode_bitmap);
            sb_info->nr_free_inodes--;
            return ino;
        }
    }
    return -ENOSPC;
}

// 尋找連續 N 個空閒區塊
int osfs_alloc_extent(struct osfs_sb_info *sb_info, uint32_t nr_blocks, uint32_t *start_block) {
    uint32_t i, j, count;
    for (i = 0; i <= sb_info->block_count - nr_blocks; i++) {
        count = 0;
        for (j = 0; j < nr_blocks; j++) {
            if (test_bit(i + j, sb_info->block_bitmap)) break;
            count++;
        }
        if (count == nr_blocks) {
            for (j = 0; j < nr_blocks; j++) set_bit(i + j, sb_info->block_bitmap);
            sb_info->nr_free_blocks -= nr_blocks;
            *start_block = i;
            pr_info("osfs: Allocated extent: start_block=%u, nr_blocks=%u\n", i, nr_blocks);
            return 0;
        }
    }
    return -ENOSPC;
}

struct inode *osfs_iget(struct super_block *sb, unsigned long ino) {
    struct osfs_inode *oi = osfs_get_osfs_inode(sb, ino);
    struct inode *inode;
    if (!oi) return ERR_PTR(-EFAULT);

    inode = new_inode(sb);
    if (!inode) return ERR_PTR(-ENOMEM);

    inode->i_ino = ino;
    inode->i_sb = sb;
    inode->i_mode = oi->i_mode;
    inode->i_size = oi->i_size;
    inode->i_private = oi;

    if (S_ISDIR(inode->i_mode)) {
        inode->i_op = &osfs_dir_inode_operations;
        inode->i_fop = &osfs_dir_operations;
    } else {
        inode->i_op = &osfs_file_inode_operations;
        inode->i_fop = &osfs_file_operations;
    }
    
    insert_inode_hash(inode);
    return inode;
}