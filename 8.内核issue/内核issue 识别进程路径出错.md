### 识别进程路径出错

> 病毒在运行之后将自己的运行程序加密 导致读取出来的路径不对

1.分析出进程的函数名

首先，使用 `ps` 命令或其他工具查找进程的 PID：

```
ps aux | grep <进程名>
```

或者直接使用 `pidof` 命令：

```
php
pidof <进程名>
```

然后，在 `/proc/<PID>/exe` 目录中查找进程的可执行文件路径，其中 `<PID>` 是第一步中获取到的进程 ID：

```
bash
ls -l /proc/<PID>/exe
```

这个命令会输出一个符号链接，指向进程的可执行文件路径。

或者，使用 `readlink` 命令来获取符号链接指向的路径：

```
bash
```

1. ```
   readlink /proc/<PID>/exe
   ```

   这个命令会直接输出进程的可执行文件路径。

注意：这种方法只能在进程还在运行时获取进程的路径。如果进程已经退出，那么对应的 `/proc` 目录也会消失，无法再获取进程的路径。





##### 简易的内核ps命令

```C++
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>

static int __init procinfo_init(void)
{
    struct task_struct *task;

    printk(KERN_INFO "--------------------------\n");

    for_each_process(task) {
        printk(KERN_INFO "Process: %s (pid = %d)\n", task->comm, task->pid);
        //printk(KERN_INFO "State: %ld\n", task->state);
    }

    printk(KERN_INFO "--------------------------\n");
    
    return 0;
}

static void __exit procinfo_exit(void)
{
    printk(KERN_INFO "Procinfo module removed\n");
}

module_init(procinfo_init);
module_exit(procinfo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");
```

这个内核模块的功能是遍历系统中的所有进程，并输出每个进程的状态和运行命令。具体实现中，我们通过定义一个 `struct task_struct` 类型的指针来访问进程描述符，然后使用 `for_each_process` 循环遍历所有进程，输出每个进程的信息。



`for_each_process` 是一个 Linux 内核宏，用于遍历系统中的所有进程。它定义在 `include/linux/sched.h` 头文件中，其实现代码如下：

```
c
#define for_each_process(p) \
        for (p = &init_task ; (p = next_task(p)) != &init_task ; )
```

`for_each_process` 宏接受一个 `struct task_struct` 类型的指针作为参数，用于遍历系统中的所有进程。在宏的实现中，它先将指针 `p` 指向系统中的第一个进程，即 `init_task`，然后通过 `next_task` 函数获取下一个进程的指针，直到遍历完所有的进程为止。

需要注意的是，`init_task` 是 Linux 系统中的一个特殊进程，它是整个进程树的根节点。在遍历进程时，我们从 `init_task` 开始遍历，并且在遍历到 `init_task` 时停止，这样可以避免遍历到已经被释放的进程或者非法进程。

在内核编程中，`for_each_process` 宏是非常常用的，可以用于实现各种与进程相关的功能，例如遍历进程列表、统计进程数量、查询进程状态等。



