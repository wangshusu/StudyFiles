# 系统学习windows驱动

## 1.windows驱动环境

![image-20230409195417632](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409204055.png)



### 1.1 windows项目设置

![image-20230409193901154](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409204053.png)

2.关闭

![image-20230409193951585](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409204051.png)

3.关闭

![image-20230409194005434](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409204048.png)



## 操作系统和指针

![image-20230409195714225](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409204046.png)

最简单的驱动

![image-20230409200037405](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409204044.png)





### 驱动对象与设备对象

![image-20230409205745699](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409205745.png)

![image-20230409203936432](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409204039.png)

修改IRP主函数中的一些指向

![image-20230409205418724](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409205418.png)

![image-20230409205656733](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409205656.png)



驱动对象详解 ：

```C++
typedef struct _DRIVER_OBJECT {
  CSHORT          Type;
  CSHORT          Size;
  PDEVICE_OBJECT  DeviceObject;
  ULONG           Flags;
  PVOID           DriverStart;
  ULONG           DriverSize;
  PVOID           DriverSection;
  PDRIVER_EXTENSION DriverExtension;
  UNICODE_STRING  DriverName;
  PUNICODE_STRING HardwareDatabase;
  PFAST_IO_DISPATCH FastIoDispatch;
  PDRIVER_INITIALIZE DriverInit;
  PDRIVER_STARTIO DriverStartIo;
  PDRIVER_UNLOAD DriverUnload;
  PDRIVER_DISPATCH MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];
} DRIVER_OBJECT, *PDRIVER_OBJECT;

```

- `Type`： 标识该结构体类型，始终为 `IO_TYPE_DRIVER`。
- `Size`： 结构体大小（字节数），在驱动程序开始时由操作系统初始化，用于指示结构体的实际大小。
- `DeviceObject`： 驱动程序所控制的设备对象链表中第一个设备对象的指针，也即最上层设备对象指针。
- `Flags`： 驱动程序对象的标志位，用于指示该驱动程序的特性和属性。
- `DriverStart`： 驱动程序代码段的起始地址。
- `DriverSize`： 驱动程序代码段在内存中占用的大小。
- `DriverSection`： 驱动程序代码段所在的内存区段（MS-DOS兼容节）。
- `DriverExtension`： 驱动程序扩展结构体指针，可以通过该指针访问到更详细的驱动信息。
- `DriverName`： 驱动程序名称，以UNICODE字符串形式表示。
- `HardwareDatabase`： 指向硬件数据库路径字符串的指针，该路径可以由隐藏文件SYSTEM.DAT或SOFTWARE.DAT指定。
- `FastIoDispatch`： 快速I/O分派表指针，它是一组函数指针表，将I/O请求直接转发到驱动程序中实现。
- `DriverInit`： 驱动初始化函数指针，当驱动程序被加载时由操作系统调用。
- `DriverStartIo`： 启动I/O处理函数指针，当设备对象上有IRP请求等待被处理时由操作系统调用。
- `DriverUnload`： 驱动卸载函数指针，当驱动程序需要被卸载时由操作系统调用。
- `MajorFunction[]`： 驱动程序支持的所有IRP主函数（Major Function）的函数指针数组，这些函数由驱动程序自身提供。



`MajorFunction`是Windows驱动程序开发中的一个重要结构，表示驱动程序可以支持的所有IRP主函数（major function）。每个主函数对应着一种I/O请求类型，因此驱动程序需要实现与每种请求类型对应的主函数来响应I/O请求。下面是常用的主函数：

- `IRP_MJ_CREATE`： 处理创建文件请求。
- `IRP_MJ_CLOSE`： 处理关闭文件请求。
- `IRP_MJ_CLEANUP`： 处理清除文件上下文环境的请求。
- `IRP_MJ_DEVICE_CONTROL`： 处理设备控制请求，通常与用户空间进程进行交互。
- `IRP_MJ_READ`： 处理读取请求。
- `IRP_MJ_WRITE`： 处理写入请求。
- `IRP_MJ_FLUSH_BUFFERS`： 处理缓存刷新请求，通常在数据传输后更新缓存等待接收。
- `IRP_MJ_INTERNAL_DEVICE_CONTROL`： 处理内部设备控制请求，仅供驱动程序内部使用。
- `IRP_MJ_QUERY_INFORMATION`： 处理查询文件或目录信息的请求。
- `IRP_MJ_SET_INFORMATION`： 处理设置文件或目录信息的请求。
- `IRP_MJ_DIRECTORY_CONTROL`： 处理与文件夹相关的请求，如获取目录列表、创建目录等。
- `IRP_MJ_LOCK_CONTROL`： 处理锁定请求，通常用于处理流式介质上的锁定。



设备对象结构体

```C++
typedef struct _DEVICE_OBJECT {
  CSHORT                    Type;
  USHORT                    Size;
  LONG                      ReferenceCount;
  struct _DRIVER_OBJECT     *DriverObject;
  struct _DEVICE_OBJECT     *NextDevice;
  struct _DEVICE_OBJECT     *AttachedDevice;
  struct _IRP               *CurrentIrp;
  PIO_TIMER                 Timer;                 // 等待时间戳
  ULONG_PTR                 DeviceExtensionSize;   // 设备扩展大小
  struct _DEVICE_OBJECT     *ParentDevice;         // 父设备对象指针
  struct _DEVICE_OBJECT     *OverlayingDevice;     // 叠加的设备对象指针
  PVPB                      Vpb;                   // 卷参数块指针（如果存在）
  ULONG                     Flags;                 // 标志位
  ULONG                     Characteristics;       // 设备特性
  __volatile PVOID          DeviceQueue;           // 设备队列头
  ULONG                     AlignmentRequirement;  // 对齐需求
  KDEVICE_QUEUE             DeviceQueueEntry;      // 队列头
  KDPC                      Dpc;                   // DPC对象，用于异步处理
  ULONG                     ActiveThreadCount;     // 活动线程计数
  PSECURITY_DESCRIPTOR     SecurityDescriptor;    // 安全性描述符
  KEVENT                    DeviceLock;            // 设备锁事件，用于同步访问设备对象
  USHORT                    SectorSize;            // 磁盘扇区大小
  USHORT                    Spare1;
  struct _DEVOBJ_EXTENSION   *DeviceObjectExtension;  // 设备对象扩展指针
  PVOID                     Reserved;              // 预留
} DEVICE_OBJECT, *PDEVICE_OBJECT;

```

- `Type`： 标识该结构体类型，始终为 `IO_TYPE_DEVICE`。
- `Size`： 结构体大小（字节数），在创建设备对象时由操作系统初始化。
- `ReferenceCount`： 引用计数器，表示有多少个内核对象正在使用该设备对象。
- `DriverObject`： 包括该设备对象的驱动程序对象。
- `NextDevice`： 驱动程序与设备对象之间的链表。
- `AttachedDevice`： 指向链中下一个设备对象的指针。
- `CurrentIrp`： 指向当前IRP请求的指针。
- `Timer`： 当前设备等待时间戳，如果超时则调用超时处理函数。
- `DeviceExtensionSize`： 扩展设备结构体大小，允许驱动程序为设备对象附加额外的信息。
- `ParentDevice`： 指向父设备对象的指针。
- `OverlayingDevice`： 指向叠加设备对象的指针。
- `Vpb`： 卷参数块指针，仅当装载了文件系统或卷管理器时才存在。
- `Flags`： 标志位，用于指示该设备对象的特性和属性。
- `Characteristics`： 设备特性，用于描述该设备所支持的功能和属性。
- `DeviceQueue`： 设备队列头，包含处理等待IRP请求的线程队列。
- `AlignmentRequirement`： 对齐需求，指明该设备要求数据的对齐方式。
- `DeviceQueueEntry`： 一个KDEVICE_QUEUE类型结构体，用来表示I/O请求队列。
- `Dpc`： DPC对象指针，用于异步处理。
- `ActiveThreadCount`： 活动线程计数器，用于记录当前正在处理设备请求的活动线程数量。
- `SecurityDescriptor`： 安全性描述符，用于控制对该设备对象的访问权限。
- `DeviceLock`： 设备锁事件，用于同步访问设备对象。
- `SectorSize`： 磁盘扇区大小。
- `Spare1`： 预留字段。
- `DeviceObjectExtension`： 设备对象扩展结构体指针。
- `Reserved`： 预留字段。



设备管理器安装驱动 

>  如果安装不成功就是需要取消数字签名
>
> 设置->重置->F7

操作->扫描->添加过时硬件

![image-20230409211527207](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409211527.png)



创建KMCF(带inf文件使用设备管理器进行安装的)驱动文件需要进行操作

![image-20230409211818475](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409211818.png)

![image-20230409211839044](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409211839.png)

![image-20230409211903015](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409211903.png)





### 内存的管理与操作

![image-20230409212849818](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409212849.png)

在Windows内核中，申请堆内存可以通过函数`ExAllocatePoolWithTag`进行实现。该函数的原型如下：

DiffCopyInsertNew

```C
PVOID ExAllocatePoolWithTag(
  POOL_TYPE PoolType,
  SIZE_T    NumberOfBytes,
  ULONG     Tag
);
```

其中，`PoolType`表示请求分配的内存池类型，包括`NonPagedPool`、`PagedPool`、`NonPagedPoolNx`和`PagedPoolCacheAligned`等；`NumberOfBytes`表示所需内存字节数；`Tag`是自定义的标识符，用于跟踪内存分配情况。

以下是一个例子，演示了如何在驱动程序中使用`ExAllocatePoolWithTag`分配内存：

DiffCopyInsertNew

```C
#define MY_TAG 'mytg'

void *pMem = ExAllocatePoolWithTag(NonPagedPool, sizeof(my_struct), MY_TAG);
if (pMem == NULL) {
    return STATUS_INSUFFICIENT_RESOURCES;
}
//对分配的内存进行初始化
RtlZeroMemory(pMem, sizeof(my_struct));
```

释放内存可以通过函数`ExFreePool`实现。该函数的原型如下：

DiffCopyInsertNew

```C
VOID ExFreePool(
  PVOID P
);
```

其中，`P`是指向待释放内存的指针。需要注意的是，在释放内存之前，应该先取消对内存的操作，避免访问非法内存。

以下是一个例子，演示了如何在驱动程序中使用`ExFreePool`释放内存：

DiffCopyInsertNew

```C
if (pMem != NULL) {
    //取消所有对内存的操作
    RtlZeroMemory(pMem, sizeof(my_struct));
    //释放内存
    ExFreePool(pMem);
    pMem = NULL;
}
```



堆内存的清零操作可以使用函数`RtlZeroMemory`或者`memset`来实现。这些函数可以将一段内存区域中的所有字节都设置为指定的值，通常用于初始化内存。

以下是`RtlZeroMemory`和`memset`的实现方式及用法：

1. `RtlZeroMemory`:

DiffCopyInsertNew

```C
void RtlZeroMemory(
  void *Dst,
  size_t Length
);
```

其中，`Dst`表示待清零的内存区域，`Length`表示内存区域的大小，单位是字节。

以下是一个例子，演示了如何使用`RtlZeroMemory`清零内存：

DiffCopyInsertNew

```C
#define MY_TAG 'mytg'

void *pMem = ExAllocatePoolWithTag(NonPagedPool, sizeof(my_struct), MY_TAG);
if (pMem == NULL) {
    return STATUS_INSUFFICIENT_RESOURCES;
}
//对分配的内存进行初始化
RtlZeroMemory(pMem, sizeof(my_struct));
```

1. `memset`:

DiffCopyInsertNew

```C
void *memset(
  void  *s,
  int   c,
  size_t n
);
```

其中，`s`表示待清零的内存区域，`c`表示要设置的值，`n`表示内存区域的大小，单位是字节。

以下是一个例子，演示了如何使用`memset`清零内存：

DiffCopyInsertNew

```C
#define MY_TAG 'mytg'

void *pMem = ExAllocatePoolWithTag(NonPagedPool, sizeof(my_struct), MY_TAG);
if (pMem == NULL) {
    return STATUS_INSUFFICIENT_RESOURCES;
}
//使用memset清零内存
memset(pMem, 0, sizeof(my_struct));
```



![image-20230409213407441](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409213407.png)





![image-20230409214435208](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409214435.png)

![image-20230409214920587](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409214920.png)

![image-20230409215325051](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/20230409215325.png)