### 内核hook

> 内核头文件存在 /user/sec/kernels/
>
> 如果没有则需要安装相应的 kernel-devel 和 kernel-headers 包，才能编译内核驱动程序

通过命令安装 :

```shell
yum install kernel-devel kernel-headers
```

通过下载安装包安装:

```shell
rmp -ivh kernel-devel-
```



##### kprobes hook

> 使用 kprobes：可以使用 kprobes 技术，将 hook 函数嵌入到内核中，并利用动态跟踪机制，在系统调用执行前或执行后执行自定义的操作。

```C
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
```

在这个示例代码中，我们首先定义了一个 kprobe 结构体 `kp`，并将它的 `symbol_name` 成员设置为 "sys_open"，即要 hook 的系统调用名字。接着，我们定义了一个 kprobe 执行的 hook 函数 `handler_pre`，在这个函数中，我们可以编写自定义操作。例如，在上面的示例代码中，我们只是简单地输出了一条信息，表示 hook 成功。在 hook 函数执行完后，需要返回一个值，如果返回 0，则表示 hook 操作不中断原始系统调用的执行。

在模块初始化函数 `my_init` 中，我们将 kprobe 结构体中的 `pre_handler` 成员设置为我们定义的 hook 函数 `handler_pre`，并调用 `register_kprobe` 函数注册 kprobe。在模块清理函数 `my_exit` 中，我们则使用 `unregister_kprobe` 函数取消注册 kprobe。

最后，我们需要在模块的头部添加必要的头文件，并设置模块的许可证、作者和描述信息。当模块加载时，kprobe 就会生效，hook open 系统调用，并在执行前输出一条信息。



在 Linux 内核中，函数 open 的原型定义在头文件 `include/linux/fs.h` 中，其原型如下：

```
cCopy code
struct file *open(const char *, int, umode_t);
```

其中，参数含义如下：

- `const char *filename`：打开的文件名。
- `int flags`：打开文件的方式，包括 O_RDONLY，O_WRONLY，O_RDWR 等。
- `umode_t mode`：文件的权限。

该函数的返回值为一个指向 struct file 结构体的指针，用于操作打开的文件。



Kprobe 是 Linux 内核提供的一种动态跟踪机制，它允许开发人员在不影响系统正常运行的情况下，向内核函数中插入代码进行跟踪。以下是 kprobe 结构体中常用的成员：

- addr：要跟踪的内核函数的地址。
- pre_handler：在内核函数执行前调用的处理函数，用于执行自定义的操作。
- post_handler：在内核函数执行后调用的处理函数，用于执行自定义的操作。
- fault_handler：在跟踪过程中发生异常时调用的处理函数，用于执行自定义的错误处理操作。
- symbol_name：要跟踪的内核函数的符号名称。
- probe_private：传递给处理函数的私有数据。
- maxactive：允许在一个时间点上激活的 kprobe 实例的最大数量。

使用 kprobe 可以在运行时动态地跟踪内核函数，通过 pre_handler 和 post_handler 可以在内核函数执行前后执行自定义的操作，从而实现 hook 的效果。

对于 kprobe 结构体中的 pre_handler 成员来说，如果不指定处理函数，它的默认值是 NULL。如果在注册 kprobe 时不指定 pre_handler，kprobe 不会执行任何操作，只会单纯地将自己插入到要跟踪的内核函数中。



> Makefile

```C
# Makefile for kprobe_demo
DIR := /usr/src/kernels/3.10.0-1160.88.1.el7.x86_64

# The target module
obj-m := kprobes.o

# Path to the kernel source
ifndef DIR
KDIR := /usr/src/kernels/$(shell uname -r)/
else 
KDIR := $(DIR)
endif

# Current directory
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
```

下面是各个参数的含义：

- `make`: 告诉系统执行 make 命令。
- `-C /usr/src/kernels/$(shell uname -r)/`: 指定 Makefile 文件的路径，这里指定的是当前系统使用的内核版本对应的内核源代码的路径。
- `M=$(PWD)`: 指定了当前目录是内核模块的源码所在目录。
- `modules`: 告诉 make 命令只编译内核模块，而不编译内核本身。