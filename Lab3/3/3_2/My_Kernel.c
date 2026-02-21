#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <asm/current.h>

#define procfs_name "Mythread_info"
#define BUFSIZE  2048
char buf[BUFSIZE]; //kernel buffer

static ssize_t Mywrite(struct file *fileptr, const char __user *ubuf, size_t buffer_len, loff_t *offset){

    size_t len = (buffer_len < BUFSIZE) ? buffer_len : BUFSIZE - 1;

    if (copy_from_user(buf, ubuf, len)) return -EFAULT;

    buf[len] = '\0';
    return buffer_len;
}

static ssize_t Myread(struct file *fileptr, char __user *ubuf, size_t buffer_len, loff_t *offset){
    char temp_buf[BUFSIZE];
    int len;

    if (*offset > 0) return 0;

    len = snprintf(temp_buf, BUFSIZE, "%sPID: %d, TID: %d, time: %llu\n", 
                   buf, current->tgid, current->pid, 
                   (unsigned long long)(current->utime / 100 / 1000));

    if (copy_to_user(ubuf, temp_buf, len)) return -EFAULT;

    *offset = len;

    return len;
}



static struct proc_ops Myops = {
    .proc_read = Myread,
    .proc_write = Mywrite,
};

static int My_Kernel_Init(void){
    proc_create(procfs_name, 0644, NULL, &Myops);   
    pr_info("My kernel says Hi");
    return 0;
}

static void My_Kernel_Exit(void){
    remove_proc_entry("Mythread_info", NULL);
    pr_info("My kernel says GOODBYE");
}

module_init(My_Kernel_Init);
module_exit(My_Kernel_Exit);

MODULE_LICENSE("GPL");
