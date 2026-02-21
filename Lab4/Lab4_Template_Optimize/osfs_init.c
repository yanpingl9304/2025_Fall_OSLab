#include <linux/init.h>
#include <linux/module.h>
#include "osfs.h"

/**
 * Function: osfs_mount
 * Description: 掛載 osfs 檔案系統。
 * 使用 mount_nodev 因為這是一個基於記憶體的檔案系統，不需要實體設備。
 */
static struct dentry *osfs_mount(struct file_system_type *fs_type,
                                 int flags,
                                 const char *dev_name,
                                 void *data)
{
    // 調用 super.c 中的 osfs_fill_super 來初始化超級塊
    return mount_nodev(fs_type, flags, data, osfs_fill_super);
}

/**
 * Function: osfs_kill_superblock
 * Description: 當檔案系統被卸載 (umount) 時，清理並釋放記憶體。
 */
static void osfs_kill_superblock(struct super_block *sb)
{
    struct osfs_sb_info *sb_info = sb->s_fs_info;

    pr_info("osfs: 正在卸載檔案系統並釋放資源\n");

    if (sb_info) {
        // 釋放在 osfs_fill_super 中透過 vmalloc 分配的連續記憶體塊
        // 包含 sb_info, bitmaps, inode_table 和 data_blocks
        vfree(sb_info);
        sb->s_fs_info = NULL;
    }

    // 呼叫核心函式處理剩餘的超級塊清理作業
    kill_litter_super(sb);
    
    pr_info("osfs: 檔案系統已成功卸載\n");
}

/**
 * 定義檔案系統類型結構
 */
struct file_system_type osfs_type = {
    .owner = THIS_MODULE,
    .name = "osfs",
    .mount = osfs_mount,
    .kill_sb = osfs_kill_superblock,
    .fs_flags = FS_USERNS_MOUNT, // 允許在 User Namespace 中掛載
};

/**
 * 模組初始化：註冊檔案系統
 */
static int __init osfs_init(void)
{
    int ret;

    ret = register_filesystem(&osfs_type);
    if (ret) {
        pr_err("osfs: 註冊檔案系統失敗\n");
        return ret;
    }

    pr_info("osfs: 模組已載入並註冊成功\n");
    return 0;
}

/**
 * 模組退出：註銷檔案系統
 */
static void __exit osfs_exit(void)
{
    int ret;

    ret = unregister_filesystem(&osfs_type);
    if (ret)
        pr_err("osfs: 註銷檔案系統失敗\n");
    else
        pr_info("osfs: 模組已卸載並註銷成功\n");
}

module_init(osfs_init);
module_exit(osfs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OSLAB");
MODULE_DESCRIPTION("A memory-based file system with Extent allocation strategy");