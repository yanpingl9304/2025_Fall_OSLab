#include <linux/fs.h>
#include "osfs.h"

static struct dentry *osfs_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags) {
    struct osfs_sb_info *sb_info = dir->i_sb->s_fs_info;
    struct osfs_inode *parent_oi = dir->i_private;
    struct osfs_dir_entry *entries;
    int i, count;

    if (parent_oi->i_extent.ee_len == 0) return NULL;

    entries = (struct osfs_dir_entry *)(sb_info->data_blocks + parent_oi->i_extent.ee_start * BLOCK_SIZE);
    count = parent_oi->i_size / sizeof(struct osfs_dir_entry);

    for (i = 0; i < count; i++) {
        if (strncmp(entries[i].filename, dentry->d_name.name, dentry->d_name.len) == 0 &&
            strlen(entries[i].filename) == dentry->d_name.len) {
            return d_splice_alias(osfs_iget(dir->i_sb, entries[i].inode_no), dentry);
        }
    }
    return NULL;
}

static int osfs_create(struct mnt_idmap *idmap, struct inode *dir, struct dentry *dentry, umode_t mode, bool excl) {
    struct super_block *sb = dir->i_sb;
    struct osfs_sb_info *sb_info = sb->s_fs_info;
    struct inode *inode;
    struct osfs_inode *oi, *parent_oi = dir->i_private;
    int ino = osfs_get_free_inode(sb_info);

    if (ino < 0) return -ENOSPC;

    inode = new_inode(sb);
    inode->i_ino = ino;
    inode->i_mode = mode;
    simple_inode_init_ts(inode);

    oi = osfs_get_osfs_inode(sb, ino);
    memset(oi, 0, sizeof(*oi));
    oi->i_ino = ino;
    oi->i_mode = mode;
    inode->i_private = oi;

    // 為新檔案分配 1 塊 Extent
    if (osfs_alloc_extent(sb_info, 5, &oi->i_extent.ee_start) == 0) {
        oi->i_extent.ee_len = 5;
        oi->i_blocks = 5;
    }

    struct osfs_dir_entry *entries = (struct osfs_dir_entry *)(sb_info->data_blocks + parent_oi->i_extent.ee_start * BLOCK_SIZE);
    int idx = parent_oi->i_size / sizeof(struct osfs_dir_entry);
    
    strncpy(entries[idx].filename, dentry->d_name.name, dentry->d_name.len);
    entries[idx].inode_no = ino;
    parent_oi->i_size += sizeof(struct osfs_dir_entry);

    d_instantiate(dentry, inode);
    return 0;
}

static int osfs_iterate(struct file *filp, struct dir_context *ctx) {
    struct inode *inode = file_inode(filp);
    struct osfs_sb_info *sb_info = inode->i_sb->s_fs_info;
    struct osfs_inode *oi = inode->i_private;
    struct osfs_dir_entry *entries;

    if (ctx->pos == 0 && !dir_emit_dots(filp, ctx)) return 0;
    
    entries = (struct osfs_dir_entry *)(sb_info->data_blocks + oi->i_extent.ee_start * BLOCK_SIZE);
    int i = ctx->pos - 2;
    int count = oi->i_size / sizeof(struct osfs_dir_entry);

    for (; i < count; i++) {
        if (!dir_emit(ctx, entries[i].filename, strlen(entries[i].filename), entries[i].inode_no, DT_UNKNOWN))
            break;
        ctx->pos++;
    }
    return 0;
}

const struct inode_operations osfs_dir_inode_operations = {
    .lookup = osfs_lookup,
    .create = osfs_create,
};

const struct file_operations osfs_dir_operations = {
    .iterate_shared = osfs_iterate,
};