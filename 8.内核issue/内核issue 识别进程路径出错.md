### 1.识别进程路径出错

> 病毒在运行之后将自己的运行程序加密 导致读取出来的路径不对

1.分析出进程的函数名

首先，使用 `ps` 命令或其他工具查找进程的 PID：

```shell
ps aux | grep <进程名>
```

或者直接使用 `pidof` 命令：

```shell
pidof <进程名>
```

然后，在 `/proc/<PID>/exe` 目录中查找进程的可执行文件路径，其中 `<PID>` 是第一步中获取到的进程 ID：

```shell
ls -l /proc/<PID>/exe
```

这个命令会输出一个符号链接，指向进程的可执行文件路径。

或者，使用 `readlink` 命令来获取符号链接指向的路径：


 ```shell
readlink /proc/<PID>/exe
 ```

  这个命令会直接输出进程的可执行文件路径。

注意：这种方法只能在进程还在运行时获取进程的路径。如果进程已经退出，那么对应的 `/proc` 目录也会消失，无法再获取进程的路径。





### 2.简易的内核ps命令

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

```c
#define for_each_process(p) \
        for (p = &init_task ; (p = next_task(p)) != &init_task ; )
```

`for_each_process` 宏接受一个 `struct task_struct` 类型的指针作为参数，用于遍历系统中的所有进程。在宏的实现中，它先将指针 `p` 指向系统中的第一个进程，即 `init_task`，然后通过 `next_task` 函数获取下一个进程的指针，直到遍历完所有的进程为止。

需要注意的是，`init_task` 是 Linux 系统中的一个特殊进程，它是整个进程树的根节点。在遍历进程时，我们从 `init_task` 开始遍历，并且在遍历到 `init_task` 时停止，这样可以避免遍历到已经被释放的进程或者非法进程。

在内核编程中，`for_each_process` 宏是非常常用的，可以用于实现各种与进程相关的功能，例如遍历进程列表、统计进程数量



### 3.读取/proc目录下的文件获取绝对路径

> 我们发现病毒将 /proc/<pid>/exe 对应的链接隐藏起来了 不得不通过别的方法来获取到文件的绝对路径
>
> 通过 /proc/<pid>/ 目录下的其他文件来获取绝对文件名 
>
> 简单的介绍一下该目录下的文件主要记录的内容

- cmdline: 记录了进程启动时的命令行参数
- environ: 记录了进程环境变量
- fd/: 目录下的文件记录了进程打开的文件描述符
- maps: 记录了进程内存映射信息，包括进程加载的所有共享库的路径
- mem: 可以通过读写该文件实现对进程内存的操作
- root: 记录了进程的根目录
- stat: 记录了进程的状态信息，如进程ID、父进程ID、进程状态、进程开始时间等
- status: 记录了进程的详细状态信息，如进程内存使用情况、CPU占用情况、进程限制等

这里通过读取 maps 下的信息来获取文件名 

![image-20230328234037396](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230328234037.png)

>  上图可以看出这里是存放了一些数据的 其中就包括了 **程序运行的绝对路径**

>  接下来就是提取这个绝对路径 在这之前了解一下 maps 中的信息有哪些

`/proc/<pid>/maps` 文件中记录了进程地址空间的内存映射信息，每行都对应一个地址区间和一个映射的文件或者匿名映射。每一行的格式如下：

```c
codeaddress       perms offset   dev   inode   pathname
00400000-00452000 r-xp  00000000 08:01 1471464 /usr/bin/dbus-daemon
```

其中各字段的含义为：

- `address`：进程的地址空间中某个区域的起始地址和结束地址。该字段由两个十六进制数值组成，中间用短横线 `-` 分隔。例如，上面示例中的地址区间为 `00400000-00452000`。

- `perms`

    ：该区域的访问权限和状态。其中包含如下四个字符：

    - `r`：可读；
    - `w`：可写；
    - `x`：可执行；
    - `p`：私有，即该区域只能被当前进程访问。 其中各字符的含义可以组合使用，例如 `r-xp` 表示该区域可读、可执行，但不可写，且为私有区域。

- `offset`：该区域在映射文件中的偏移量。如果该区域不是由文件映射而来，那么该字段为 0。

- `dev`：该区域所在文件的设备号。

- `inode`：该区域所在文件的 inode 编号。

- `pathname`：该区域所在文件的路径名。如果该区域是由匿名映射创建的，则该字段为 `[anon]`。

在这些信息中，可执行文件的路径名可以从 `pathname` 字段中获得，这对于获取正在运行的进程的可执行文件路径非常有用。

```C
char *get_executable_path(int pid, char *path, size_t path_length) {
    char maps_path[PATH_MAX];

    // 构造 maps 文件路径
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    // 打开 maps 文件
    FILE* fp = fopen(maps_path, "r");
    if (fp == NULL) {
        perror("fopen failed");
        return NULL;
    }

    // 解析 maps 文件中的可执行文件路径
    char* line = NULL;
    size_t line_size = 0;
    ssize_t line_len;
    char* exe_path = NULL;
    while ((line_len = getline(&line, &line_size, fp)) != -1) {
        if (strstr(line, " r-xp ") != NULL) {  // 可执行文件的映射区域
            char* path_start = strchr(line, '/');
            char* path_end = strchr(line, '\n');
            if (path_start != NULL && path_end != NULL) {
                *path_end = '\0';
                if (strlen(path_start) < path_length) {
                    strncpy(path, path_start, path_length);
                    exe_path = realpath(path, NULL);
                    if (exe_path == NULL) {
                        fprintf(stderr, "realpath failed: %s\n", strerror(errno));
                    }
                } else {
                    fprintf(stderr, "Executable path too long\n");
                }
                break;
            }
        }
    }

    free(line);
    fclose(fp);
    return exe_path;
}
```

