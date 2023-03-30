/*
 * @FilePath: \undefinedd:\桌面\kprobes.c
 * @Brief: 
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

/* 定义 kprobe 结构体 */
static struct kprobe kp = {
    .symbol_name    = "sys_open",
};

/* 定义 kprobe 执行的 hook 函数 */
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    /* 执行自定义操作 */
    printk(KERN_INFO "Open system call hooked!\n");

    /* 返回 0，表示 hook 操作不中断原始系统调用的执行 */
    return 0;
}

/* 模块初始化函数 */
static int __init my_init(void)
{
    int ret;

    /* 注册 kprobe */
    kp.pre_handler = handler_pre;
    ret = register_kprobe(&kp);
    if (ret < 0) {
        printk(KERN_INFO "Failed to register kprobe\n");
        return -1;
    }

    printk(KERN_INFO "kprobe registered successfully!\n");

    return 0;
}

/* 模块清理函数 */
static void __exit my_exit(void)
{
    /* 取消注册 kprobe */
    unregister_kprobe(&kp);

    printk(KERN_INFO "kprobe unregistered successfully!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wzx");
MODULE_DESCRIPTION("kprobe example");
