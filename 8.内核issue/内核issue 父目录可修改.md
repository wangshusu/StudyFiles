### 内核issue

1.拉取代码 更新子模块 切换分支 

>这里的每个子模块都需要更换到对应的分支后 然后拉取代码 确保其中的分支不为空

2.将代码拉入编译环境中 

> 直接将代码复制到编译环境  后序在编译环境中编译代码  将生成的驱动代码放到对应的机器中就行

3.驱动路径

> 驱动路径 : /opt/ArwClient_3.0.230321.0188_x64/bin





#### 2.简单的内核代码

##### 2.1 编写简单的hello.c内核代码

```C
/*
 * a simple kernel module: hello
 *
 * Copyright (C) 2020 xxx (xxxxx)
 *
 * Licensed under GPLv2 or later
 */
#include <linux/init.h>
#include <linux/module.h>

static int __init hello_init(void)
{
        printk(KERN_INFO "hello World enter\n");
        return 0;
}

module_init(hello_init);

static void __exit hello_exit(void)
{
        printk(KERN_INFO "hello World exit\n");
}
module_exit(hello_exit);

MODULE_AUTHOR("xx xx <xxxxx>");//作者
MODULE_LICENSE("GPS v2");//模块许可证声明,一般用GPL v2
MODULE_DESCRIPTION("A simple hello world module");//模块描述
MODULE_ALIAS("a simplest module"); //别名
```

##### 2.2 编写Makefile

```c
KVERS = $(shell uname -r)

obj-m += hello.o

build: kernel_modules

kernel_modules:
        make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
        make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean

```



##### 2.3 在hello_world目录下执行make命令

> 会生成 hello.ko 文件

```makefile
$ make
make -C /lib/modules/3.10.0Likaige+/build M=/home/LiKaige/kernel/2_linux-3.10/drivers/misc/hello_world modules
make[1]: Entering directory '/home/LiKaige/kernel/2_linux-3.10'
  CC [M]  /home/LiKaige/kernel/2_linux-3.10/drivers/misc/hello_world/hello.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /home/LiKaige/kernel/2_linux-3.10/drivers/misc/hello_world/hello.mod.o
  LD [M]  /home/LiKaige/kernel/2_linux-3.10/drivers/misc/hello_world/hello.ko
make[1]: Leaving directory '/home/LiKaige/kernel/2_linux-3.10'
$ ls *.ko
hello.ko
```



##### 2.4 常用的几个命令

```C
modprobe			   //加载模块自动解决依赖关系(注意这里无需输入.ko后缀）
dmesg				   //dmesg 命令主要用来显示内核信息。
insmod hello.ko        //加载模块
rmmod hello.ko         //卸载模块
lsmod | grep hello     //查看模块是否运行及是否被使用
tail /var/log/messages //查看加载或卸载模块时内核打印的log
modinfo hello.ko       //查看内核模块参数,如author, license, description等
```



要查看已经编译好的驱动 .ko 文件的信息，可以使用以下命令：

```
modinfo 驱动文件路径
```

其中，驱动文件路径为编译好的驱动文件的路径。该命令会输出驱动程序的信息，包括驱动程序的版本、编译时间、作者等。如果需要查看已加载的驱动的信息，可以使用以下命令：

```c
lsmod //模块名、模块大小、模块使用次数、模块依赖关系
```

该命令会列出所有已加载的驱动的信息，包括驱动名、驱动大小、使用次数等。如果只想查看某个驱动的信息，可以使用以下命令：

```
modinfo 驱动名
```

其中，驱动名为驱动文件的文件名去掉后缀 .ko。



##### 2.5 判断驱动是否可以加载当前系统

使用 `lsmod` 列出所有的驱动信息

使用 ` modinfo` 分别查看 上述列出驱动的任意一个 和 将要加载的驱动

其中可以查看已经安装的驱动的信息  和 即将安装的驱动的信息 如果下面的前面三个数字相同 就是可以加载成功的

> 也可使用  ` uname -r` 命令 

```C
uname [-amnrsv][--help][--version]
```

- -a 或--all 　显示全部的信息，包括内核名称、主机名、操作系统版本、处理器类型和硬件架构等。。
- -m 或--machine 　显示处理器类型。
- -n 或--nodename 　显示主机名。
- -r 或--release 　显示内核版本号。
- -s 或--sysname 　显示操作系统名称。
- -v 　显示操作系统的版本。
- --help 　显示帮助。
- --version 　显示版本信息。
- -p 显示处理器类型（与 -m 选项相同）。



##### 卸载服务

chkconfig 

chkconfig ArwClient off







### issue

[windows文件系统浅谈](https://bbs.kanxue.com/thread-270140.htm)

[linux下readlink函数详解](https://blog.csdn.net/feixue0000/article/details/14167333)

[Linux chattr命令](https://www.runoob.com/linux/linux-comm-chattr.html)

[linuxps命令](https://www.jianshu.com/p/943b90150c10)

[linux下proc目录](https://zhuanlan.zhihu.com/p/26923061)

