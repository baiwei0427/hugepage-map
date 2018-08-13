#include <linux/module.h>

#define MODULE_NAME "hugepage driver"

int init_module(void)
{
        printk(KERN_INFO "Install %s\n", MODULE_NAME);
        return 0;
}


void cleanup_module(void)
{       
        printk(KERN_INFO "Remove %s\n", MODULE_NAME);
}

MODULE_LICENSE("GPL");


