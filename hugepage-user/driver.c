#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/mm.h>

#define DEV_NAME "hugepage-driver"
#define DEV_MAJOR 42

static struct class *dev_class = NULL;

static int dev_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "%s\n", __FUNCTION__);
        return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "%s\n", __FUNCTION__);
        return 0;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
        int     res;
        struct  page *page;
        unsigned int *vir_addr = NULL, *phy_addr = NULL;

        printk(KERN_INFO "%s\n", __FUNCTION__);
        
        down_read(&current->mm->mmap_sem);
        // Pin one user page in memory
        res = get_user_pages(current, 
                             current->mm,
                             (unsigned long)buf,
                             1,
                             1,              
                             1,
                             &page,
                             NULL);

        if (res) {
                printk(KERN_INFO "Got mmaped.\n");
                vir_addr = (unsigned int*)kmap(page);
                phy_addr = (unsigned int*)virt_to_phys((void*)phy_addr);
                printk(KERN_INFO "Virtual address is %p\n", vir_addr);
                printk(KERN_INFO "Physical address is %p\n", phy_addr);
                *vir_addr = *vir_addr + 1;
                kunmap(page);
                page_cache_release(page);
        } else {
                printk(KERN_INFO "Fail to mmap\n");
        }

        up_read(&current->mm->mmap_sem);

        return 0;
}

static struct   file_operations dev_ops = {
        .owner  = THIS_MODULE,
        .open   = dev_open,
        .release = dev_release,
        .write  = dev_write
};

int init_module(void)
{
        int ret;

        printk(KERN_INFO "Install %s\n", DEV_NAME);

        ret = register_chrdev(DEV_MAJOR, DEV_NAME, &dev_ops);
        dev_class = class_create(THIS_MODULE, DEV_NAME);
        device_create(dev_class, NULL, MKDEV(DEV_MAJOR, 0), NULL, DEV_NAME);

        return (ret);
}


void cleanup_module(void)
{       
        device_destroy(dev_class, MKDEV(DEV_MAJOR, 0));
        class_destroy(dev_class);
        unregister_chrdev(DEV_MAJOR, DEV_NAME);

        printk(KERN_INFO "Remove %s\n", DEV_NAME);
}

MODULE_LICENSE("GPL");


