### 内核issue 无法挂载驱动

1.重启显示 无法找到 符号 __fentry__

![image-20230327120934803](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230327120934803.png)

2.查找内核头文件中是否 存在 __fentry__

在 2.6.32-431.20.3.el6.x86_64 中未找到

在 3.8.13-44.1.1.el6uek.x86_64找到

![image-20230327121727921](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230327121727921.png)

查看 内核文件中 ./scripts/recordmcount.h 

` cat ./scripts/recordmcount.h | head -n 300 | tail -n +250`

![image-20230327121848903](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230327121848903.png)

> 代码解释:

这段代码是用来查找给定的 `relp` 对应的符号表项（symbol table entry），并判断是否符合 `mcount` 或 `__fentry__` 的命名规则。其中：

- `sym0` 是 ELF 文件的符号表（symbol table）。
- `relp` 是一个指向重定位表（relocation table）项的指针。
- `str0` 是符号表字符串表（symbol table string table）。

函数 `get_mcountsym` 的主要作用是查找符号表中是否有符合条件的符号。如果找到了，则返回该符号表项的索引（`mcountsym`）。

`char const *fentry = "__fentry__";` 的作用是定义了一个字符串变量 `fentry`，其值为 `__fentry__`。这个字符串用于检查是否存在 `__fentry__` 这个符号。在 Linux 内核中，`__fentry__` 是由编译器添加的一个标记，在函数开头添加一个指针，在函数返回前将指针置为 NULL。这个标记被用于函数追踪（function tracing）以及安全性检查等用途。在这段代码中，如果找到了一个符号名称为 `__fentry__` 的符号，则将其索引（`Elf_r_sym(relp)`）赋值给 `mcountsym`。

> 如何取消 __ fentry __

要在编译时禁用 `__fentry__`，可以使用编译器选项 `-fno-ret-protector` 或 `-fno-stack-protector`。这两个选项分别用于禁用返回地址保护和堆栈保护，这两种保护机制都使用了 `__fentry__` 标记。

具体来说，`-fno-ret-protector` 选项禁用了编译器在函数结尾处插入的一段代码，用于检查函数返回地址的合法性，这个代码会调用名为 `__stack_chk_fail` 的函数来触发错误处理。而 `-fno-stack-protector` 选项则禁用了编译器在函数开头处插入的一段代码，用于检查堆栈是否溢出的情况。

这两个选项都可以在编译命令中加入，例如：

```
r
gcc -fno-ret-protector -c file.c
```

或者

```
arduino
gcc -fno-stack-protector -c file.c
```

这样，在编译代码时就会禁用 `__fentry__` 标记，从而取消函数追踪和安全性检查等机制。需要注意的是，禁用这些保护机制可能会降低代码的安全性，因此应该仔细考虑是否需要这样做。



### 使用ios挂载安装 gcc

> 直接挂载安装 

[如何在Linux安装gcc](https://blog.csdn.net/maoyuanming0806/article/details/77075493)

> 使用命令在 虚拟机中挂载 share 中的共享文件夹

```C++
30  ip a
31  lsblk
32  mount -l
33  file /dev/sr0
34  mkdir /mnt/share
35  mkdir /mnt/iso
36  mkdir /mnt/sr0
37  mount -t cifs -o username=test,password=f,iocharset=utf8 //172.16.1.12/iso /mnt/share/
```

1. `ip a`：这个命令用于列出虚拟机的网络接口和它们的IP地址。这个命令可以用来确认虚拟机是否连接到了网络并且有可用的IP地址。
2. `lsblk`：这个命令用于列出虚拟机上的块设备。在这里，我们使用它来确认ISO映像将要挂载到哪个设备上。
3. `mount -l`：这个命令用于列出当前已经挂载的文件系统。这可以用来确认我们将要挂载的ISO映像还没有被挂载。
4. `file /dev/sr0`：这个命令用于检查 /dev/sr0 这个设备文件所对应的设备类型。在这里，我们使用它来确认这个设备是一个光盘设备，因为我们将要挂载一个ISO映像。
5. `mkdir /mnt/share`：这个命令用于创建一个用来挂载共享目录的挂载点。
6. `mkdir /mnt/iso`：这个命令用于创建一个用来挂载ISO映像的挂载点。
7. `mkdir /mnt/sr0`：这个命令用于创建一个用来挂载光盘设备的挂载点。
8. `mount -t cifs -o username=test,password=f,iocharset=utf8 //172.16.1.12/iso /mnt/share/`：这个命令用于将一个共享目录挂载到虚拟机的文件系统中。在这里，我们将共享目录中的ISO映像文件挂载到 /mnt/share/ 目录中。

在这个过程中，我们首先创建了三个挂载点： /mnt/share/、/mnt/iso/ 和 /mnt/sr0/。然后，我们将共享目录挂载到 /mnt/share/ 目录中。最后，我们使用挂载点 /mnt/share/ 中的ISO映像文件挂载到 /mnt/iso/ 目录中，以便进行GCC的安装。

```C++
38  cd /mnt/share/
   39  ls
   40  cd linux/
   41  ls
   42  cd Orale_linux
   43  ls
   44  cd oracle_linux_6.6/
   45  ls
   46  ll
   47  mount OracleLinux_6.6_x64.iso /mnt/iso/
   48  mount OracleLinux_6.6_x64.iso /mnt/iso/ -l loop
49  mount OracleLinux_6.6_x64.iso /mnt/iso/ -o loop
   50  cd /mnt/iso
   51  ls
   52  find .|grep gcc
   53  cd Packages/
   54  rpm -i gcc-4.4.7-11.el6.x86_64.rpm
   55  rpm -i gcc-c++-4.4.7-11.el6.x86_64.rpm
56  find .|grep c++
   57  rpm -i libstdc++-4.4.7-11.el6.x86_64.rpm
   58  rpm -i libstdc++-devel-4.4.7-11.el6.x86_64.rpm
   59  rpm -i gcc-c++-4.4.7-11.el6.x86_64.rpm
   60  gcc
61  ls |grep -i kernel
   62  rpm -i kernel-devel-2.6.32-504.el6.x86_64.rpm
   63  rpm -i kernel-headers-2.6.32-504.el6.x86_64.rpm
```

1. `ll`：这个命令列出当前目录中的文件和目录，并显示它们的详细信息，包括权限、所有者、组、大小、修改日期和时间等。
2. `mount OracleLinux_6.6_x64.iso /mnt/iso/`：这个命令将OracleLinux_6.6_x64.iso文件挂载到/mnt/iso目录下。
3. `mount OracleLinux_6.6_x64.iso /mnt/iso/ -l loop`：这个命令将OracleLinux_6.6_x64.iso文件以loop方式挂载到/mnt/iso目录下。
4. `mount OracleLinux_6.6_x64.iso /mnt/iso/ -o loop`：这个命令将OracleLinux_6.6_x64.iso文件以loop方式挂载到/mnt/iso目录下，并指定使用loop设备。
5. `cd /mnt/iso`：进入/mnt/iso目录。
6. `ls`：列出/mnt/iso目录中的文件和目录。
7. `find .|grep gcc`：这个命令在/mnt/iso目录及其子目录中查找包含"gcc"关键字的文件。
8. `cd Packages/`：进入/mnt/iso/Packages/目录。
9. `rpm -i gcc-4.4.7-11.el6.x86_64.rpm`：这个命令安装名为gcc-4.4.7-11.el6.x86_64.rpm的软件包。
10. `rpm -i gcc-c++-4.4.7-11.el6.x86_64.rpm`：这个命令安装名为gcc-c++-4.4.7-11.el6.x86_64.rpm的软件包。
11. `find .|grep c++`：这个命令在/mnt/iso目录及其子目录中查找包含"c++"关键字的文件。
12. `rpm -i libstdc++-4.4.7-11.el6.x86_64.rpm`：这个命令安装名为libstdc++-4.4.7-11.el6.x86_64.rpm的软件包。
13. `rpm -i libstdc++-devel-4.4.7-11.el6.x86_64.rpm`：这个命令安装名为libstdc++-devel-4.4.7-11.el6.x86_64.rpm的软件包。
14. `gcc`：这个命令检查是否已经成功安装了gcc。
15. `ls |grep -i kernel`：这个命令列出/mnt/iso目录中包含"kernel"关键字的文件。
16. `rpm -i kernel-devel-2.6.32-504.el6.x86_64.rpm`：这个命令安装名为kernel-devel-2.6.32-504.el6.x86_64.rpm的软件包。
17. `rpm -i kernel-headers-2.6.32-504.el6.x86_64.rpm`：这个命令安装名为kernel-headers-2.6.





### 虚拟机网盘设置静态IP地址

首先，使用 root 用户登录到 Oracle 虚拟机。

执行以下命令来打开 /etc/network/interfaces 文件并备份原始文件：

```bash
sudo cp /etc/network/interfaces /etc/network/interfaces.bak
sudo nano /etc/network/interfaces
```

如果您喜欢其他编辑器，可以将 "nano" 替换为您喜欢的编辑器。

使用以下命令修改 eth0 的配置：

```bash
sudo ifconfig eth0 <静态IP地址> netmask <子网掩码> up
sudo route add default gw <默认网关地址>

sudo ifconfig th0 172.16.32.122/16
```

将 <静态IP地址>、<子网掩码>和 <默认网关地址> 替换为您想要使用的值。

检查是否成功配置了静态IP地址。您可以使用以下命令检查：

```bash
ip addr show eth0
```

如果IP地址已成功分配，您应该能够在输出中看到类似以下内容：

```bash
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
   link/ether 08:00:27:af:13:8f brd ff:ff:ff:ff:ff:ff
   inet 192.168.1.100/24 brd 192.168.1.255 scope global eth0
      valid_lft forever preferred_lft forever
   inet6 fe80::a00:27ff:feaf:138f/64 scope link
      valid_lft forever preferred_lft forever
```

注意，这里的IP地址应该是您刚刚配置的静态IP地址。





### 简单的hello world驱动

> hello.c

```C
#include <linux/init.h>
#include <linux/module.h>
 
static int __init hello_init(void) {
    printk(KERN_ALERT "Hello, world\n");
    return 0;
}
 
static void __exit hello_exit(void) {
    printk(KERN_ALERT "Goodbye, cruel world\n");
}
 
MODULE_LICENSE("Dual BSD/GPL");
 
module_init(hello_init);
module_exit(hello_exit);
```



> Makefile

```makefile
obj-m := hello.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
    make -C $(KDIR) M=$(PWD) modules

clean:
    make -C $(KDIR) M=$(PWD) clean

```





### trance命令

