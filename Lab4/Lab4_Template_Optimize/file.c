#include <linux/fs.h>
#include <linux/uaccess.h>
#include "osfs.h"

static ssize_t osfs_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos) {
    struct inode *inode = file_inode(filp);
    struct osfs_inode *oi = inode->i_private;
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;

    if (*ppos >= oi->i_size || oi->i_extent.ee_len == 0) return 0;
    if (*ppos + len > oi->i_size) len = oi->i_size - *ppos;

    // Extent 起始位置計算偏移
    void *data_ptr = sb_info->data_blocks + (oi->i_extent.ee_start * BLOCK_SIZE) + *ppos;

    if (copy_to_user(buf, data_ptr, len)) return -EFAULT;
    
    *ppos += len;
    return len;
}

static ssize_t osfs_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos) {
    struct inode *inode = file_inode(filp);
    struct osfs_inode *oi = inode->i_private;
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;

    // 如果未分配空間，分配固定 5 個區塊的 Extent
    if (oi->i_extent.ee_len == 0) {
        if (osfs_alloc_extent(sb_info, 5, &oi->i_extent.ee_start)) return -ENOSPC;
        oi->i_extent.ee_len = 5;
        pr_info("osfs: First write - trigger extent allocation: start=%u, len=%u\n", 
            oi->i_extent.ee_start, oi->i_extent.ee_len);
    }

    if (*ppos + len > oi->i_extent.ee_len * BLOCK_SIZE)
        len = (oi->i_extent.ee_len * BLOCK_SIZE) - *ppos;

    void *data_ptr = sb_info->data_blocks + (oi->i_extent.ee_start * BLOCK_SIZE) + *ppos;

    if (copy_from_user(data_ptr, buf, len)) return -EFAULT;

    *ppos += len;
    if (*ppos > oi->i_size) {
        oi->i_size = *ppos;
        inode->i_size = *ppos;
    }
    mark_inode_dirty(inode);
    return len;
}

const struct file_operations osfs_file_operations = {
    .read = osfs_read,
    .write = osfs_write,
    .llseek = default_llseek,
};

const struct inode_operations osfs_file_inode_operations = {};