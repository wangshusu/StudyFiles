## windows大佬博客

## 1. 驱动基础知识

驱动程序是操作系统和硬件通讯的桥梁，同时，驱动程序可以实现很多特殊功能，比如，虚拟光驱（虚拟设备），内核级hook，文件系统透明加密（过滤驱动），修改Windows内核等等

并非所有驱动程序都必须由设备的设计方编写。如果设备根据已发布的硬件标准来设计。这时驱动程序可以由 Microsoft 编写，设备设计者无须提供驱动程序。

并非所有的驱动程序都与硬件关联。某些驱动程序与任何硬件设备根本不关联。 例如，如果需要访问内核，或者实现虚拟设备，过滤设备等等，需要通过编写驱动程序实现。提供一个用户模式下运行且提供用户界面的应用程序，还需要在内核模式下运行且可以访问核心操作系统数据的驱动程序。这个驱动程序可以称为“软件驱动程序”。软件驱动程序与硬件设备不关联。还有个例子是一些屏幕录像软件通过驱动程序直接读取屏幕，而不是从应用层调用API截图，实现硬件加速。

在正式开始介绍驱动开发之前，我们需要先来了解一下基本的概念。

1. 驱动程序的运行级别

以内核模式驱动程序为例，用户模式驱动是后来才有的。

其实intel 80x86提供了四个运行级别，R0，R1，R2和R3，现代主流操作系统，Windows自然也不例外，只用了R0和R3两个运行级别，分别为“内核模式”和“用户模式”。应用程序运行在用户模式，驱动程序运行在内核模式。

这也就是说，驱动程序在运行的那一刻，就是天道！驱动程序中的指令，就是天道法则！（打个比方），所以说驱动程序运行时理论上可以做任何事情，RootKit病毒和杀毒软件都是利用的驱动程序。

2。Windows驱动程序的种类

windows9x时代，是VxD驱动，从NT的第一代开始到现在的Windows 10，windows逐渐出现了三种驱动程序模型，它们分别是：

1。遗留驱动：NT驱动程序：这是基础的驱动模型，也是本系列博文着重介绍的驱动模型。

2。WDM驱动：为了支持设备的热插拔（比如USB设备），微软推出了WDM驱动，支持电源管理和即插即用

3。WDF驱动：有人说，如果WDM是Win32，WDF就是MFC。这个比喻还是比较恰当的。WDF驱动其实就是微软又封装了一些更方便的内核函数，使驱动开发更加便捷。另外WDF还提供了两套机制，一种是内核模式驱动程序，一种是用户模式驱动程序。

3。虚拟内存（虚拟地址空间）

驱动程序可以访问整个4GB的虚拟内存，用户程序只允许访问低2G。

关于虚拟内存的分页原理和Windows的内存管理机制，这里不再介绍，有兴趣的可以查阅相关资料

4。API的处理

在Win32子系统调用的API，会进一步调用在ntdll里的Native API，Native API进入内核，调用系统的服务例程，被I/O管理器生成IRP并发送到设备栈最上层的设备对象所在驱动程序的派遣函数中，有可能会进一步向下层的驱动程序继续发送（比如过滤驱动），驱动程序完成任务。

5。同步与异步

我对同步和异步的理解，可能不是很准确：
同步：调用一个接口后，一直等待到被调用者处理完毕，再返回
异步：调用一个接口后，不等待到被调用者处理完毕，直接返回，处理其他东西。

同步处理是驱动开发中的重要任务。Windows是支持异步操作的操作系统，编写驱动程序时，如果有必要，需要进行同步处理。

6。 设备对象和设备堆栈

“设备对象”为 DEVICE_OBJECT 结构的一个实例。设备堆栈是一系列设备形成的和栈结构类似的东西，简单说，磁盘设备之上可能有磁盘过滤设备，再上面是卷设备，再上面可能有文件系统过滤设备，等等，IRP总是z先发送到设备堆栈最上面一层的设备中。

7。符号连接

一般的，对于NT驱动而言，应用程序不直接访问设备，而是访问设备的符号连接，比如第一个磁盘卷的设备名是

“\Device\HarddiskVolume1”

其符号连接为：内核模式下
"\??\C:"

用户模式下：



而我们的应用程序访问的时候都是用符号连接。

对于WDM驱动，一般不使用符号连接，而是使用设备接口。



## 2. 简单的NT驱动

一、驱动入口

驱动程序的入口点是DriverEntry，此函数的原型是：

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,PUNICODE_STRING pRegistryPath) 
参数1：系统发来的驱动对象的指针
参数2：一个内核字符串对象的指针，是驱动的服务名称。（NT驱动是以服务的形式安装的，关于服务的知识可以看我以前的服务开发和SCManager的相关博文）一般这个参数是用于在加载时判断服务名称是不是驱动程序所接受的（如果有这方面的需要）。关于内核字符串我将在之后的博文中讲。

返回值：返回内核状态码，STATUS_SUCCESS表示成功

二、注册驱动卸载函数

在驱动入口中可以注册驱动卸载函数，如果不注册，驱动程序一旦加载就无法卸载（rootkit病毒、杀毒软件的驱动无法卸载除了hook卸载函数外也可以用这种方法阻止卸载）

注册的方法为：

	pDriverObject->DriverUnload = DriverUnload;
驱动卸载函数原型：

extern "C" VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
应该在这个函数中完成驱动程序的清理工作，以免内存泄露等问题。

三、派遣函数

派遣函数类似于win32应用程序的回调函数，只不过派遣函数的调用是并发的，因为调用派遣函数的线程不是固定的，win32程序的回调函数是在一个线程里工作的，是串行的。

通过这些代码注册派遣函数：

```C++
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchRoutine;
```

这里注册了创建、关闭、读、写四种类型的IRP的派遣函数。除了这四个以外，还有个非常常用的是IRP_MJ_DEVICE_CONTROL，应用程序调用DeviceIoControl时I/O管理器发送此IRP。

IRP有很多，比如：（这里面还有一些是给WDM驱动用于即插即和电源管理的）

```C++
IRP_MJ_CLEANUP
IRP_MJ_CLOSE
IRP_MJ_CREATE
IRP_MJ_DEVICE_CONTROL
IRP_MJ_FILE_SYSTEM_CONTROL
IRP_MJ_FLUSH_BUFFERS
IRP_MJ_INTERNAL_DEVICE_CONTROL
IRP_MJ_PNP
IRP_MJ_POWER
IRP_MJ_QUERY_INFORMATION
IRP_MJ_READ
IRP_MJ_SET_INFORMATION
IRP_MJ_SHUTDOWN
IRP_MJ_SYSTEM_CONTROL
IRP_MJ_WRITE
```

派遣函数中，我们什么也不做，因为对于这个驱动空壳而言，没有要做的，将IRP完成，直接返回成功。

其中关于IRP请求的我会在下面的博文中讲。

```C++
extern "C" NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
```

完整代码：

```C++
#include <ntddk.h>
extern "C" VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject);
extern "C" NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp);
 
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,PUNICODE_STRING pRegistryPath)//驱动入口
{
	DbgPrint("DriverEntry\r\n");
 
	pDriverObject->DriverUnload = DriverUnload;//注册卸载函数
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchRoutine;//注册派遣函数
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchRoutine;
	
	return STATUS_SUCCESS;
}
 
extern "C" VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("DriverUnload\r\n");
}
 
extern "C" NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);//完成IRP请求
	return status;
}
```

其中DbgPrint是输出调试信息。


### 4. 驱动程序的编译

参考其它文章中总结的



## 3. 使用SCM加载NT驱动（用应用程序加载驱动）
这一篇中，将介绍如何自己编写一个驱动程序加载器，即使用编程的方法加载驱动，而不是使用工具。

我在上一篇中也提到了，驱动程序是通过服务加载的，我以前写过SCM（服务控制管理器）API的文章，详细介绍各函数的具体用法，详见：

[Win32] SCManager 服务控制管理器API（1）：http://blog.csdn.net/zuishikonghuan/article/details/47803033
[Win32] SCManager 服务控制管理器API（2）：http://blog.csdn.net/zuishikonghuan/article/details/47808805

加载NT驱动的基本步骤是：

1。获取SCM服务控制管理器句柄。
2。用CreateService创建驱动服务。
3。用OpenService获取服务句柄。
4。用StartService启动驱动服务。
5。用ControlService停止驱动服务。

启动驱动服务除了用StartService外，还有很多“非官方，非正式”的方法，比如使用Zw/NtLoadDriver，Zw/NtSetSystemInformation等

好了废话不多说，加载上一篇中的驱动，加载器源码如下，驱动为C:\mydriver.sys，驱动服务名为mydriver

```C++
#include <Windows.h>
 
BOOL WINAPI InstallService(LPCTSTR ServiceName,
	LPCTSTR DisplayName,
	LPCTSTR szPath,
	DWORD StartType,
	DWORD ErrorControl = SERVICE_ERROR_NORMAL,
	DWORD ServiceType = SERVICE_WIN32_OWN_PROCESS){
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (hSCManager != NULL){
		SC_HANDLE hService = CreateService(hSCManager, ServiceName, DisplayName, SERVICE_ALL_ACCESS, ServiceType, StartType, ErrorControl, szPath, NULL, NULL, NULL, NULL, TEXT(""));
		if (hService != NULL){
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);
			return TRUE;
		}
		CloseServiceHandle(hSCManager);
		return FALSE;
	}
	else return FALSE;
}
BOOL WINAPI DeleteService(LPCTSTR ServiceName){
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (hSCManager != NULL){
		SC_HANDLE hService = OpenService(hSCManager, ServiceName, DELETE);
		if (hService != NULL){
			if (DeleteService(hService))
			{
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return TRUE;
			}
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);
			return FALSE;
		}
		CloseServiceHandle(hSCManager);
		return FALSE;
	}
	else return FALSE;
}
BOOL WINAPI StartService2(LPCTSTR ServiceName){
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (hSCManager != NULL){
		SC_HANDLE hService = OpenService(hSCManager, ServiceName, SERVICE_START);
		if (hService != NULL){
			if (StartService(hService, 0, NULL))
			{
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return TRUE;
			}
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);
			return FALSE;
		}
		CloseServiceHandle(hSCManager);
		return FALSE;
	}
	else return FALSE;
}
BOOL WINAPI StopService(LPCTSTR ServiceName){
	
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	SERVICE_STATUS svcsta = { 0 };
	if (hSCManager != NULL){
		SC_HANDLE hService = OpenService(hSCManager, ServiceName, SERVICE_STOP);
		if (hService != NULL){
			if (ControlService(hService, SERVICE_CONTROL_STOP, &svcsta))
			{
				CloseServiceHandle(hService);
				CloseServiceHandle(hSCManager);
				return TRUE;
			}
			CloseServiceHandle(hService);
			CloseServiceHandle(hSCManager);
			return FALSE;
		}
		CloseServiceHandle(hSCManager);
		return FALSE;
	}
	else return FALSE;
}
int _tmain(int argc, _TCHAR* argv[])
{
	InstallService(TEXT("mydriver"), TEXT("mydriver"), TEXT("C:\\mydriver.sys"), SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, SERVICE_KERNEL_DRIVER);
	//SERVICE_KERNEL_DRIVER：驱动服务；SERVICE_DEMAND_START：手动启动
	if (StartService2(TEXT("mydriver"))){
		printf("驱动已经加载\n");
	}
	Sleep(3000);
	if (StopService(TEXT("mydriver"))){
		printf("驱动已经停止\n");
	}
	getchar();
	return 0;
}
```



## 4. 内核中的内存分配和错误码

在驱动开发中，我们不应该使用C/C++运行时函数中的malloc或者calloc函数分配内存，更不应该使用new关键字，因为内核中的内存分配需要特殊处理。

在应用程序中，每个应用都有2G的虚拟内存，因此内存并不紧张，而所有的驱动程序共用内核模式的2G虚拟内存，因此内核中的资源非常宝贵，应该尽量节省。

更可怕的是内存泄露，应用程序即使发生了内存泄露，在其结束时操作系统可以通过进程上下文中的虚拟内存页面映射关系，释放这一部分虚拟内存对应的物理内存，但是驱动程序如果申请虚拟内存后不释放，结果就是在系统重新启动之前这一部分地址所在的内存页面一直处于“被申请未释放”的状态，想想那种情景吧。如果驱动程序申请的虚拟内存位于非页内存池，那么是不能交换到磁盘文件中的，因此这一部分虚拟内存对应的物理内存也一直无法空闲出来，后果更加严重。

内核中申请虚拟内存的函数一般有这么几个：ExAllocatePool（WithTag）、ExAllocatePoolWithQuota（Tag）、ExFreePool（WithTag），我们来看看这两个ExAllocatePool（申请内核虚拟内存）和ExFreePool（释放内核虚拟内存），其他的都差不多，具体可以看MSDN。

```C++
PVOID ExAllocatePool(
  _In_ POOL_TYPE PoolType,
  _In_ SIZE_T    NumberOfBytes
);
```

PoolType：指定的池内存分配的类型。请参阅 POOL_TYPE。



常用的有： 

```C++
NonPagedPool：从非分页内存池中分配虚拟内存 
PagedPool：从分页内存池中分配虚拟内存 
```

NumberOfBytes：指定要分配的字节数。
返回值：成功返回分配的内存指针，失败返回NULL。

```C++
VOID ExFreePool(
  _In_ PVOID P
);
```

P：指定要释放的内存块的地址。
还有一点时间，说说内核中的错误码NTSTATUS吧

32位系统下，NTSTATUS是unsigned long类型，内核中的函数的返回值以及我们以后处理派遣函数等等都会用到。

其中，STATUS_SUCCESS表示成功 ，可以用NT_SUCCESS宏判断是否是STATUS_SUCCESS 。

自然还有很多其他的错误码，比如

STATUS_UNSUCCESS（不成功）

STATUS_ACCESS_DENIED（访问被拒绝）

STATUS_IN_PAGE_ERROR（页故障）
等等



## 5. 内核中的字符串

在驱动开发中，内核函数使用的字符串不再是应用程序使用的Win32子系统API和Native API中的char*和wchar_t*，而是内核Unicode字符串UNICODE_STRING。

内核字符串有两种，ANSI字符串是ANSI_STRING，Unicode字符串是UNICODE_STRING，他们的结构是这样定义的：

```C++
typedef struct _STRING {
  USHORT Length;
  USHORT MaximumLength;
  PCHAR  Buffer;
} ANSI_STRING, *PANSI_STRING;
```

```C++
typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
```

Length：字符串长度，都是以字节数为单位的！而不是字符数。

MaximumLength：字符串的最大长度，即 Buffer 指向的缓冲区长度，字节为单位。

Buffer：真实字符串的指针。

下面，以Unicode版本，介绍一下内核字符串的创建、释放、复制、比较等

**创建字符串：RtlInitUnicodeString**

```C++
VOID RtlInitUnicodeString(
  _Out_    PUNICODE_STRING DestinationString,
  _In_opt_ PCWSTR          SourceString
);
```

DestinationString：指向要初始化的UNICODE_STRING结构的指针。



SourceString：一个指向以 null 结尾的宽字符字符串，用于初始化DestinationString。

**释放字符串：RtlFreeUnicodeString**

```C++
VOID RtlFreeUnicodeString(
  _Inout_ PUNICODE_STRING UnicodeString
);
```

UnicodeString：指向以前分配的、要释放的 Unicode 字符串UNICODE_STRING结构的指针。



**复制字符串：RtlCopyString**

```C++
VOID RtlCopyString(
  _Out_          PSTRING DestinationString,
  _In_opt_ const STRING  *SourceString
)
```

DestinationString：指向目标字符串的指针。
SourceString：指向源字符串的指针。

注：
1。这里的字符串ANSI_STRING结构和UNICODE_STRING结构均可。
2。目标字符串结构中的缓冲区一定要事先申请好，从 SourceString 复制的字节数是 SourceString 的长度或 DestinationString 的最大长度，以较小者为准。

**字符串比较：RtlCompareUnicodeString**

```C++
LONG RtlCompareUnicodeString(
  _In_ PCUNICODE_STRING String1,
  _In_ PCUNICODE_STRING String2,
  _In_ BOOLEAN          CaseInSensitive
);
```

String1：指向第一个字符串UNICODE_STRING结构的指针
String2：指向第二个字符串UNICODE_STRING结构的指针。

CaseInSensitive：如果为 TRUE，则在做比较时应忽略大小写。

返回值：RtlCompareUnicodeString 返回比较的结果:
0：String1 等于 String2。
<0：String1 小于 String2。
>0：String1 大于 String2。

另外还有几个比较常用的，但在这里不再详细说了，都很简单。

字符串编码转换：RtlAnsiStringToUnicodeString、 RtlUnicodeStringToAnsiString
字符串转换到大写：RtlUpperString
字符串与int的转换：RtlUnicodeStringToInteger、RtlIntegerToUnicodeString



## 6. NT驱动的基本结构

下面，将会以此源码为基础进行扩充，并给大家解说一下NT驱动的基本结构。其实WDM和KWDF驱动也是基于此的。

1。驱动对象 DRIVER_OBJECT

驱动的入口函数 DriverEntry 的第一个参数就是驱动加载时系统为当前驱动创建的驱动对象的指针。

这个结构微软没有完全公开，如果有兴趣可以可以到网上找找相关资料。其中一些重要的成员有：

PDEVICE_OBJECT DeviceObject：指向第一个设备对象的指针。一个驱动可以创建多个“设备”，这个成员是指向第一个设备对象的指针。驱动程序可以使用此成员和 DEVICE_OBJECT 的 NextDevice 成员来逐句通过驱动程序创建的所有设备对象的列表。

UNICODE_STRING DriverName：驱动名称，是 UNICODE_STRING 字符串。（见我的上上篇博文“内核中的字符串”）

PDRIVER_DISPATCH MajorFunction：派遣函数的指针。应将他视作一个数组，通过他注册派遣函数，如何注册我已经在之前的那一篇博文“驱动开发（2）第一个NT驱动和NT驱动的编译”中详细说了。

PDRIVER_UNLOAD DriverUnload：驱动卸载函数的指针。指向一个卸载函数，驱动卸载时负责清理工作，应在这个函数中关闭打开的句柄、释放申请的内存，防止内存泄露。如果驱动程序不注册卸载函数，那么驱动一旦加载就无法卸载。同样，如何注册我已经在之前的那一篇博文“驱动开发（2）第一个NT驱动和NT驱动的编译”中详细说了。

2。设备对象 DEVICE_OBJECT

结构原型：

```C++
typedef struct _DEVICE_OBJECT {
  CSHORT                      Type;
  USHORT                      Size;
  LONG                        ReferenceCount;
  struct _DRIVER_OBJECT  *DriverObject;
  struct _DEVICE_OBJECT  *NextDevice;
  struct _DEVICE_OBJECT  *AttachedDevice;
  struct _IRP  *CurrentIrp;
  PIO_TIMER                   Timer;
  ULONG                       Flags;
  ULONG                       Characteristics;
  __volatile PVPB             Vpb;
  PVOID                       DeviceExtension;
  DEVICE_TYPE                 DeviceType;
  CCHAR                       StackSize;
  union {
    LIST_ENTRY         ListEntry;
    WAIT_CONTEXT_BLOCK Wcb;
  } Queue;
  ULONG                       AlignmentRequirement;
  KDEVICE_QUEUE               DeviceQueue;
  KDPC                        Dpc;
  ULONG                       ActiveThreadCount;
  PSECURITY_DESCRIPTOR        SecurityDescriptor;
  KEVENT                      DeviceLock;
  USHORT                      SectorSize;
  USHORT                      Spare1;
  struct _DEVOBJ_EXTENSION  *  DeviceObjectExtension;
  PVOID                       Reserved;
} DEVICE_OBJECT, *PDEVICE_OBJECT;
```

说说几个重要的成员

DriverObject：指向创建此设备的驱动对象的指针

NextDevice：下一个设备对象的指针，创建过设备的驱动程序再创建设备时，创建之前最后一个设备的此成员是指向新创建的设备对象的指针。 DRIVER_OBJECT里的 DeviceObject 和此成员形成了一个类似于链表的结构，我们可以通过 DeviceObject 和 NextDevice 来枚举一个驱动创建的所有设备。

AttachedDevice：此设备附加到的设备对象的指针。关于这个，我将会在之后的过滤驱动的博文中说。

Flags：标志，比如DO_BUFFERED_IO，DO_DIRECT_IO指定设备的读写方式。

DO_BUFFERED_IO：读写此设备使用缓冲方式（系统复制缓冲区）。

DO_DIRECT_IO：读写此设备使用直接方式。

DO_EXCLUSIVE：一次只允许一个线程打开设备句柄。

DeviceExtension：设备扩展指针。创建设备时，我们需要传入设备扩展的长度，系统会分配内存，并通过这个成员让我们得到设备扩展。

DeviceType：设备类型，由IoCreateDevice等函数创建设备时指定的类型。

3。创建设备

使用 IoCreateDevice 函数创建设备，此函数的原型是：

```C++
NTSTATUS IoCreateDevice(
  _In_     PDRIVER_OBJECT  DriverObject,
  _In_     ULONG           DeviceExtensionSize,
  _In_opt_ PUNICODE_STRING DeviceName,
  _In_     DEVICE_TYPE     DeviceType,
  _In_     ULONG           DeviceCharacteristics,
  _In_     BOOLEAN         Exclusive,
  _Out_    PDEVICE_OBJECT  *DeviceObject
);
```

DriverObject：调用方的驱动对象的指针。每个驱动程序在其驱动入口例程的参数接收指向其驱动程序对象的指针。WDM 功能和筛选器驱动程序也在他们的 AddDevice 例程接收驱动程序对象指针。

DeviceExtensionSize：指定驱动程序确定要为设备扩展的设备对象分配的字节数。设备扩展的内部结构是由驱动程序定义。

设备名称：驱动名称，是 UNICODE_STRING 字符串（见我的上上篇博文“内核中的字符串”）可空。

DeviceType：设备类型，比如：FILE_DEVICE_DISK、FILE_DEVICE_KEYBOARD等等。本例中采用FILE_DEVICE_UNKNOWN，未知设备。

设备类型有很多，详见MSDN。

DeviceCharacteristics： 指定一个或多个系统定义的常数，或在一起，提供有关驱动程序的设备的其他信息。大多数驱动程序为该参数指定了FILE_DEVICE_SECURE_OPEN。

比如说：
FILE_FLOPPY_DISKETTE：指示该设备是软盘设备。
FILE_READ_ONLY_DEVICE：指示不能写入该设备。
FILE_REMOVABLE_MEDIA：指示存储设备支持可移动媒体。
等等

Exclusive：设备是否是独占的，TRUE表示独占，大多数驱动程序把此参数设置为FALSE。

DeviceObject：指向一个 PDEVICE_OBJECT 的指针，用于接收设备对象的指针。即此参数是设备对象的指针变量的指针。

返回值：成功返回STATUS_SUCCESS，失败返回指定的错误码。

设备扩展：

在驱动程序的开发中，一般不建议用全局变量，而应该使用设备扩展。创建设备时，我们需要传入设备扩展的长度，系统会分配内存，并通过返回的设备对象中的相关成员返回设备扩展的指针。

这里我们定义一个设备扩展的结构，只有一个成员，符号连接名：

```C++
typedef struct _DEVICE_EXTENSION {
	UNICODE_STRING SymLinkName;	//我们定义的设备扩展里只有一个符号链接名成员
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
```

创建设备：

```C++
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
 
	//创建设备名称的字符串
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDevice1");
 
	//创建设备
	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION), &devName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevObj);
	if (!NT_SUCCESS(status))
		return status;
 
	pDevObj->Flags |= DO_BUFFERED_IO;//将设备设置为缓冲I/O设备，关于缓冲I/O设备将会在以后的博文中讲
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到设备扩展
```

***\*4。创建符号连接\****

创建设备后，应用程序一般不能直接访问设备，因此我们需要为设备对象创建符号连接。关于符号连接的内核模式、用户模式命名的问题，请看我之前的博文“驱动开发（1）基础知识”。

创建符号连接用的是 IoCreateSymbolicLink 函数，原型为：

```C++
NTSTATUS IoCreateSymbolicLink(
  _In_ PUNICODE_STRING SymbolicLinkName,
  _In_ PUNICODE_STRING DeviceName
);
```



SymbolicLinkName：符号连接名称

DeviceName：要将符号连接绑定到的设备名称

这两个参数都是 UNICODE_STRING 字符串（见我的上上篇博文“内核中的字符串”）

返回值：成功返回STATUS_SUCCESS，失败返回指定的错误码。

创建符号连接：

```C++
//创建符号链接
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\MyDevice1_link");
	pDevExt->SymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
```

***\*5。释放资源\****

驱动程序卸载时，我们需要删除设备和符号连接。因此我们的驱动卸载函数为：

```C++
extern "C" VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("DriverUnload\r\n");
	PDEVICE_OBJECT pDevObj;
	pDevObj = pDriverObject->DeviceObject;
 
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到设备扩展
 
	//删除符号链接
	UNICODE_STRING pLinkName = pDevExt->SymLinkName;
	IoDeleteSymbolicLink(&pLinkName);
 
	//删除设备
	IoDeleteDevice(pDevObj);
}
```

这次，我们还是保留和上一次一样的派遣函数，直接完成掉。

***\*我们先编写一个Win32应用程序，很简单，打开一个设备，而且是打开我们刚刚编写的驱动程序创建的设备的符号连接。源码去下：\****

```C++
#include<Windows.h>
 
int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hfile;
	hfile = CreateFileA("\\\\.\\MyDevice1_link", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//打开设备
	if (hfile == INVALID_HANDLE_VALUE)
		MessageBoxA(0, "打开设备失败", "错误", 0);
	getchar();
	CloseHandle(hfile);
	return 0;
}
```

**驱动程序的完整源码：**

```C++
#include <ntddk.h>
extern "C" VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject);
extern "C" NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp);
 
typedef struct _DEVICE_EXTENSION {
	UNICODE_STRING SymLinkName;	//我们定义的设备扩展里只有一个符号链接名成员
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
 
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	DbgPrint("DriverEntry\r\n");
 
	pDriverObject->DriverUnload = DriverUnload;//驱动卸载函数
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchRoutine;//注册派遣函数
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchRoutine;
 
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
 
	//创建设备名称的字符串
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDevice1");
 
	//创建设备
	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION), &devName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevObj);
	if (!NT_SUCCESS(status))
		return status;
 
	pDevObj->Flags |= DO_BUFFERED_IO;//将设备设置为缓冲I/O设备，关于缓冲I/O设备将会在以后的博文中讲
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到设备扩展
 
	//创建符号链接
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\MyDevice1_link");
	pDevExt->SymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}
 
extern "C" VOID DriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("DriverUnload\r\n");
	PDEVICE_OBJECT pDevObj;
	pDevObj = pDriverObject->DeviceObject;
 
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到设备扩展
 
	//删除符号链接
	UNICODE_STRING pLinkName = pDevExt->SymLinkName;
	IoDeleteSymbolicLink(&pLinkName);
 
	//删除设备
	IoDeleteDevice(pDevObj);
}
 
extern "C" NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("DispatchRoutine\r\n");
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
```

下面我们编译这两个程序，先运行一下应用程序，果然弹出来了一个“打开设备失败”的框。

我们把驱动加载了，然后再运行应用程序，令人惊奇的是，没有弹出框，这说明应用程序成功打开了设备！这说明了应用程序可以访问我们编写的驱动了。

同时我们可以通过DebugView发现驱动程序的派遣函数被调用了2次：
![img](https://img-blog.csdn.net/20151022232230824?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

驱动开发系列的下一篇“驱动开发（7）IRP与派遣函数”将会展开这一部分的内容



## 7. IRP与派遣函数

这几篇博文，博主来说说驱动开发的一个核心内容——如何处理I/O请求。

为何要处理I/O请求？因为通常一个驱动程序是为了控制硬件而设计的（虽然没有对应具体硬件的驱动远比有对应具体硬件的驱动要多得多），可以说，是操作系统与硬件通信的桥梁。而驱动程序就是为操作系统提供了一种抽象，驱动程序如何控制硬件，是硬件厂商和硬件开发人员的工作，而操作系统只需要将I/O请求发送到驱动程序里就行了。也就是说，驱动程序向上按照微软的相关规范和标准处理Windows NT内核中的I/O请求，向下控制不同接口，不同标准的硬件。这体现了微内核的思想。（注：Windows NT内核并不是微内核，或许叫混合内核是一个不错的选择）

一个叫做“I/O请求数据包”（IRP）的数据结构在WDK的头文件中被定义，Windows NT内核就是用IRP这个结构来组织内核I/O请求的。

我们来看看IRP这个结构是怎么定义的：

```C++
typedef struct _IRP {
  .
  .
  PMDL  MdlAddress;
  ULONG  Flags;
  union {
    struct _IRP  *MasterIrp;
    .
    .
    PVOID  SystemBuffer;
  } AssociatedIrp;
  .
  .
  IO_STATUS_BLOCK  IoStatus;
  KPROCESSOR_MODE  RequestorMode;
  BOOLEAN PendingReturned;
  .
  .
  BOOLEAN  Cancel;
  KIRQL  CancelIrql;
  .
  .
  PDRIVER_CANCEL  CancelRoutine;
  PVOID UserBuffer;
  union {
    struct {
    .
    .
    union {
      KDEVICE_QUEUE_ENTRY DeviceQueueEntry;
      struct {
        PVOID  DriverContext[4];
      };
    };
    .
    .
    PETHREAD  Thread;
    .
    .
    LIST_ENTRY  ListEntry;
    .
    .
    } Overlay;
  .
  .
  } Tail;
} IRP, *PIRP;
```



暂时不解释各个成员的含义，因为现在说大家也不好理解，同时这篇博文侧重点在引人主题，是以思想为主，在接下来的几篇中，我们将围绕着处理IRP展开这些内容。

另外，还有一种说法，认为IRP数据结构只是IRP（I/O请求数据包）的一部分，还有一个IO_STACK_LOCATION的东西，这个就是I/O堆栈的当前这一层，这个可以在派遣函数中使用IoGetCurrentIrpStackLocation函数得到它的指针。关于I/O堆栈，博主不想现在详细说，只是告诉大家处理IRP时一般都要得到这一层的指针，关于I/O堆栈的细节，博主打算以后在过滤驱动的那一些博文中说。

暂时撇开如何处理IRP不谈，我们来说说派遣函数和IRP的关系。在Win32中，消息（MSG结构）有一个叫“窗口过程”（WndProc）的回调函数（CallBack Function）来处理，与之类似，在驱动开发中，IRP是在“派遣函数”或者叫“派遣例程”（DispatchRoutine）中处理的。

还记得在上一篇中，是如何注册派遣函数的吗？在DriverEntry中，用DRIVER_OBJECT中的MajorFunction指针指向的内存设置为我们派遣函数的指针，就像这样：

```C++
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)  
{  
    DbgPrint("DriverEntry\r\n");  
  
    pDriverObject->DriverUnload = DriverUnload;//驱动卸载函数  
    pDriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchRoutine;//注册派遣函数  
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchRoutine;  
    pDriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchRoutine;  
    pDriverObject->MajorFunction[IRP_MJ_READ] = DispatchRoutine; 
```

可以为每一种IRP设置一个派遣函数，也可以分开。

IRP有很多，比如：（这里面还有一些是给WDM驱动用于即插即和电源管理的）

```C++
IRP_MJ_CLEANUP
IRP_MJ_CLOSE
IRP_MJ_CREATE
IRP_MJ_DEVICE_CONTROL
IRP_MJ_FILE_SYSTEM_CONTROL
IRP_MJ_FLUSH_BUFFERS
IRP_MJ_INTERNAL_DEVICE_CONTROL
IRP_MJ_PNP
IRP_MJ_POWER
IRP_MJ_QUERY_INFORMATION
IRP_MJ_READ
IRP_MJ_SET_INFORMATION
IRP_MJ_SHUTDOWN
IRP_MJ_SYSTEM_CONTROL
IRP_MJ_WRITE
```

来看看派遣函数的原型：

```C++
extern "C" NTSTATUS DispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
```



pDevObj：设备对象的指针。也就是接受I/O请求的设备对象的指针。说白了就是我往哪个设备发出I/O请求，那么这个参数就是那个设备。

pIrp：要处理IRP的指针。

返回值：参见之前的一篇博文“内核中的内存分配和错误码”。

派遣函数类似于win32应用程序的回调函数，只不过派遣函数的调用是并发的。

在结束之前，我大致说一下下几篇博文的打算，下一篇讲一个Win32应用程序，这个应用程序呢，他将直接向磁盘这一个设备发出I/O请求，也就是绕过FileSystem直接读写磁盘上的扇区，这依旧是一个引子，让我们看看在R3下是如何向一个设备发出I/O请求的，之后就讲在驱动中如何处理这些I/O请求。﻿﻿

## 7. 处理设备I/O控制函数DeviceIoControl

在上面的两篇博文中，介绍了IRP与派遣函数，以及我们通过了一个例子“磁盘设备的绝对读写”来演示了在应用程序中是如何向一个设备发出I/O请求的。这篇博文将演示在驱动程序中处理一个非常简单的I/O请求——由DeviceIoControl这个Win32API经过一系列的调用，在内核中由I/O管理器构造生成的IRP_MJ_DEVICE_CONTROL这个IRP。

我们先来看看DeviceIoControl这个函数的原型，此函数向某个打开的设备所在驱动程序的派遣函数中发送IRP：IRP_MJ_DEVICE_CONTROL。函数原型：

```C++
BOOL WINAPI DeviceIoControl(
  _In_        HANDLE       hDevice,
  _In_        DWORD        dwIoControlCode,
  _In_opt_    LPVOID       lpInBuffer,
  _In_        DWORD        nInBufferSize,
  _Out_opt_   LPVOID       lpOutBuffer,
  _In_        DWORD        nOutBufferSize,
  _Out_opt_   LPDWORD      lpBytesReturned,
  _Inout_opt_ LPOVERLAPPED lpOverlapped
);
```

hDevice：操作是要执行的设备句柄。使用 CreateFile 函数打开。

dwIoControlCode：操作的控制代码。

需要注意的是，这个控制码不是随便定的，为了方便定义控制码，微软提供了一个CTL_CODE宏。控制码的结构如下：



本例中博主定义了这样的一个ioctl控制码：

#define IOCTL1 CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)

来看看这个宏的使用方法：
首先是这个宏的定义：

#define CTL_CODE(DeviceType, Function, Method, Access) (
  ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
DeviceType：设备类型，IoCreateDevice使用的设备类型，具体参加我之前的博文”NT驱动的基本结构“

Function：定义设备类别中的一个操作。0-2047和4096以后被微软保留，2048-4095（0x800-0xFFF）留给我们使用。

Method：定义操作模式

METHOD_BUFFERED：缓冲区方法，本例使用这种方法
METHOD_IN_DIRECT：直接输入
METHOD_OUT_DIRECT：直接输出
METHOD_NEITHER：两者都不，即其他方法
对于 Windows 嵌入式设备，此字段将被忽略。始终使用 METHOD_BUFFERED。

Access：一般用FILE_ANY_ACCESS，所有权限。

lpInBuffer：（可选）指向输入缓冲区的指针。

nInBufferSize：输入缓冲区以字节为单位的大小。

lpOutBuffer：（可选）指向输出缓冲区的指针，

nOutBufferSize：输出缓冲区以字节为单位的大小。

lpBytesReturned：（可选）指向接收“输出缓冲区中接收的数据的大小”的变量的指针。如果输出缓冲区太小，无法接收任何数据，则GetLastError返回ERROR_INSUFFICIENT_BUFFER，此时lpBytesReturned是零。如果输出缓冲区太小，不能容纳所有数据，但可以容纳一些条目，一些驱动可能将尽可能多的返回数据。在这种情况下，GetLastError返回ERROR_MORE_DATA，然后lpBytesReturned指示接收的数据量。应用程序可以指定一个新的起点再次调用DeviceIoControl。如果lpOverlapped是NULL，那么lpBytesReturned不能为 NULL。

lpOverlapped：（可选）OVERLAPPED结构的指针。如果打开hDevice时没有指定FILE_FLAG_OVERLAPPED标志，lpOverlapped将被忽略。如果打开 hDevice 时指定了FILE_FLAG_OVERLAPPED 标志，则作为异步操作执行。在这种情况下，lpOverlapped必须指向有效的重叠结构，并且必须包含事件对象的句柄。否则，该函数会失败。
注：异步操作，为 DeviceIoControl 立即返回，并且当在操作完成时终止的事件对象的操作。

返回值：如果该操作成功完成，则返回值不为零。如果操作失败，或处于挂起状态，则返回值为零。若要获取扩展的错误信息，请调用GetLastError

注意：IRP_MJ_DEVICE_CONTROL这个IRP在Win32子系统中调用DeviceIoControl生成，在NT Native层或内核模式下应该使用ZwDeviceIoControlFile。

其实这个IRP可以用于应用程序与驱动程序通信。先上应用程序的源码，还是打开我们设备的符号连接，并使用DeviceIoControl函数向驱动程序发送一个控制码，我想我不用再解释什么了，如果你不能理解，请回过头看看我的前两篇和更早的博文。

源码：

```C++
#include "stdafx.h"
#include<Windows.h>
 
#define IOCTL1 CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
 
int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE handle = CreateFileA("\\\\.\\MyDevice1_link", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE){
		MessageBoxA(0, "打开设备失败", "错误", 0);
		return 0;
	}
	unsigned char buffer[50] = { 0 };
	unsigned char buffer2[50] = { 0 };
	DWORD len;
	sprintf((char*)buffer, "hello, driver\r\n");
	if (DeviceIoControl(handle, IOCTL1, buffer, strlen((char*)buffer), buffer2, 49, &len, NULL)){
		printf("len: %d\n", len);
		for (int i = 0; i < len; i++){
			printf("0x%02X ",buffer2[i]);
		}
	}
	getchar();
	CloseHandle(handle);
	return 0;
}
```

我们再来看看驱动程序的源码，在派遣函数中处理这个IRP，我们获取了应用程序发送来的控制码后输出输入缓冲区的数据，并将输出缓冲区填充为0xF1：

```C++
#include <ntddk.h>
extern "C" VOID DriverUnload(PDRIVER_OBJECT pDriverObject);
extern "C" NTSTATUS DefDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp);
extern "C" NTSTATUS IoctlDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp);
 
#define IOCTL1 CTL_CODE(FILE_DEVICE_UNKNOWN,0x800,METHOD_BUFFERED,FILE_ANY_ACCESS)
 
typedef struct _DEVICE_EXTENSION {
	UNICODE_STRING SymLinkName;	//我们定义的设备扩展里只有一个符号链接名成员
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
 
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	DbgPrint("DriverEntry\r\n");
 
	pDriverObject->DriverUnload = DriverUnload;//注册驱动卸载函数
 
	//注册派遣函数
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlDispatchRoutine;
 
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
 
	//创建设备名称的字符串
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDevice1");
 
	//创建设备
	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION), &devName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevObj);
	if (!NT_SUCCESS(status))
		return status;
 
	pDevObj->Flags |= DO_BUFFERED_IO;//将设备设置为缓冲I/O设备，关于缓冲I/O设备将会在下一篇博文中讲！
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到设备扩展
 
	//创建符号链接
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\MyDevice1_link");
	pDevExt->SymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}
 
extern "C" VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("DriverUnload\r\n");
	PDEVICE_OBJECT pDevObj;
	pDevObj = pDriverObject->DeviceObject;
 
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到设备扩展
 
	//删除符号链接
	UNICODE_STRING pLinkName = pDevExt->SymLinkName;
	IoDeleteSymbolicLink(&pLinkName);
 
	//删除设备
	IoDeleteDevice(pDevObj);
}
 
extern "C" NTSTATUS DefDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("Enter DefDispatchRoutine\r\n");
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
extern "C" NTSTATUS IoctlDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("Enter IoctlDispatchRoutine\r\n");
	NTSTATUS status = STATUS_SUCCESS;
 
	//得到I/O堆栈的当前这一层，也就是IO_STACK_LOCATION结构的指针
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
 
	ULONG in_size = stack->Parameters.DeviceIoControl.InputBufferLength;//得到输入缓冲区的大小
	ULONG out_size = stack->Parameters.DeviceIoControl.OutputBufferLength;//得到输出缓冲区的大小
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;//得到控制码
 
	PVOID buffer = pIrp->AssociatedIrp.SystemBuffer;//得到缓冲区指针
 
	switch (code)
	{						// process request
	case IOCTL1:
		DbgPrint("====Get ioctl code 1\r\n");
		//显示输入缓冲区数据
		DbgPrint((PCSTR)buffer);
 
		//将输出缓冲区填充字符
		RtlFillMemory(buffer, out_size, 0xF1);
		break;
	default:
		status = STATUS_INVALID_VARIANT;
		//如果是没有处理的IRP，则返回STATUS_INVALID_VARIANT，这意味着用户模式的I/O函数失败，但并不会设置GetLastError
	}
 
	// 完成IRP
	pIrp->IoStatus.Status = status;//设置IRP完成状态，会设置用户模式下的GetLastError
	pIrp->IoStatus.Information = out_size;//设置操作的字节
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);//完成IRP，不增加优先级
	return status;
}
```

效果图：





这一篇中，本来打算详细解释一下处理IRP的过程，但是在这个IRP上，实在是不好讲，所以下一篇（处理缓冲I/O设备的读写请求）中介绍IRP处理的详细过程，含义，读和写都是最基本的I/O请求之一，而且也是最好理解的，所以下一篇中，详细介绍如何处理IRP以及操作系统对于缓冲I/O设备（同步模式下）读写请求的API（ReadFile（Ex）WriteFile（Ex））进入内核后的详细实现过程。﻿﻿﻿﻿

## 10.直接I/O和内存描述符表

在上一篇中，我已经详细解释了使用缓冲I/O的原因，在这里简单重复一下，如果要看详细解释请看上一篇博文。

当我们调用ReadFile(Ex)和WriteFile(Ex)读写文件，管道或者设备时，我们需要提供一个缓冲区的指针，如果是同步读（注意是同步），那么ReadFile返回后，我们要读的数据就在缓冲区里了，如果是同步写（同样，注意是同步），则WriteFile结束后，我们要写的数据就写完了，当然都是没有出错的情况下。

API调用过程，调用的Win32子系统提供的编程接口，最终调用NT Native API，nativeAPI调用KiFastSystemCall，随后进入内核，调用内核模式下的函数，当调用内核中I/O管理器的接口后，I/O管理器构造生成IRP，并发送到相应设备所在驱动程序的派遣函数中。

产生的问题是：Windows是一个多任务抢占式调度的操作系统（上世纪的Windows1.0等不是抢占式的，是多任务协作式调度的），“进程上下文”和“线程上下文”在频繁地被切换，进程上下文的切换就意味着用户模式虚拟内存（线性地址空间）的切换，我们提供的缓冲区必定是用户模式虚拟内存的一部分，因此，进入内核模式后，如果进程上下文切换了，那么我们的缓冲区指针就变成野指针了！

当应用程序打开一个设备并发出一个 I/O 请求时，会载人到驱动程序的派遣函数中，虽然此时驱动程序的 Dispatch Function 运行在调用 I/O 函数的用户模式线程上下文中，此时访问用户模式虚拟内存并不会带来影响，但现实情况总是要比理想环境复杂很多，首先，驱动程序可能异步处理 I/O 请求（异步完成 IRP 会在以后说）驱动程序会立即返回 Dispatch Function ，将 IRP 加入一个处理队列，并在之后在其他线程中完成他，此时线程上下文可能会随时被切换，从而使用户模式的虚拟内存地址变成野指针；另一种常见的情况是处理 I/O 请求的设备并非只有一个，比如文件系统驱动程序会将 IRP 发送到磁盘设备驱动程序中，而对于之后的设备的派遣函数而言，并不能确定运行在原来用户模式线程上下文中，这会带来同样的问题，即此时线程上下文可能会随时被切换，从而使用户模式的虚拟内存地址变成野指针。

为解决这个问题，因此出现了缓冲I/O设备，直接I/O设备等概念。缓冲I/O设备，就是进入内核后，操作系统分配一块内核空间虚拟内存作为缓冲区，并把用户模式虚拟内存的缓冲区中的数据复制进来，复制完成后，传给驱动程序的地址就是内核模式的地址了，内核模式虚拟内存不会因进程上下文切换而改变映射关系，这个问题就解决了。

我们的驱动程序显然是没有具体的存储硬件的，因此我们不得不寻找一些其他方法，本例中分配了一些内核虚拟内存（非页内存）当做是存储空间。

这些其实是上一篇博文中前面讲的原因的精简版本，如果你读起来感觉不清晰，建议看看我的上一篇博文。

既然缓冲I/O已经看起来很完美的解决了这个问题了，为何还要有直接I/O？

OK，正戏开始了：

我在上一篇博文中已经说过了缓冲I/O的原理，简单说，就是进入内核后，操作系统分配内核模式虚拟内存，将应用程序提供的在用户模式虚拟内存的缓冲区中数据复制到在内核模式虚拟内存的缓冲区中。内核模式虚拟内存不会因为进程上下文的切换而改变映射关系，从而驱动程序处理I/O请求时访问内核模式下的缓冲区，就解决了这个问题了。

这样做的确很简单方便，但是有一个问题：性能比较低，因为缓冲区的复制会消耗CPU时间。

因此，人们找到了优化的方案：直接I/O。

注意；除了缓冲I/O和直接I/O以外，还有一种方法叫“两者都不”（其他I/O）但是这种方法很不安全，因此不推荐使用，本文中也不打算涉及。

我们知道（不知道建议看看我上一篇博文中对虚拟内存的介绍），虚拟内存其实就是将一个“线性”的地址空间按照“页面”的形式映射到物理内存，从而实现了内存页面访问规则和进程内存空间的分离。虚拟内存是非常灵活的，于是，我们可以将用户模式虚拟内存所映射到的物理内存在内核模式重新映射一遍！我们不再需要缓冲区的复制，这极大地节省了不必要的开销，提升了驱动程序的性能。这就是直接I/O。

但事情没有想象的那么好，首先，使用直接I/O会使驱动代码更复杂，另外，在某些时候直接I/O并不能使用，比如在Windows嵌入式系统（比如Windows CE）开发时，驱动程序只能用缓冲I/O。但是我觉得，代码复杂一点无关紧要，因为就稍微复杂了一点，并不是复杂到没法阅读，Windows已经把这些都实现了，我们要做的只是调用一些接口而已。Windows（NT Kernel）还是主要在桌面市场，而且性能对于驱动程序而言非常重要。

我们还需要引人一个概念“内存描述符表”（MDL），内存描述符表是一个数据结构，我们并不需要了解他的具体内容，WDK的头文件提供了几个宏去访问他。我们需要知道的是，内存描述符表中记录了直接I/O需要的一系列内存映射数据就行了。

应用程序，发出读写请求的程序，源码：
（其实和上一篇缓冲I/O的一样，用户模式代码是不用关心设备用缓冲I/O或者直接I/O的，这里所做的修改是将用户模式缓冲区的地址输出了）

```C++
#include "stdafx.h"
#include<Windows.h>
 
int _tmain(int argc, _TCHAR* argv[])
{
	//打开设备
	HANDLE handle = CreateFileA("\\\\.\\MyDevice1_link", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE){
		MessageBoxA(0, "打开设备失败", "错误", 0);
		return 0;
	}
	unsigned char buffer1[50] = { 0 };
	unsigned char buffer2[50] = { 0 };
	DWORD len;
 
	//测试1：写入超出驱动申请的缓冲区大小的数据
	printf("1: buffer_addr: 0x%08X\n", buffer1);//输出用户模式缓冲区的地址
	if (!WriteFile(handle, buffer1, 1000, &len, NULL))
		printf("1: failed\n");//当然会失败
	
	//测试2：写入字符串hello, driver，偏移量为5
	//也就是说，跳过前五个字节再写入
	printf("2: buffer_addr: 0x%08X\n", buffer1);//输出用户模式缓冲区的地址
	sprintf((char*)buffer1, "hello, driver\r\n");
	OVERLAPPED ol = { 0 };
	ol.Offset = 5;
	if (WriteFile(handle, buffer1, strlen((char*)buffer1), &len, &ol)){
		printf("2: len: %d\n", len);
	}
 
	//测试3： 读出0-48（共49字节）的数据，并使用16进制输出
	printf("2: buffer_addr: 0x%08X\n", buffer2);//输出用户模式缓冲区的地址
	if (ReadFile(handle, buffer2, 49, &len, NULL)){
		printf("3: len: %d\n", len);
		for (int i = 0; i < len; i++){
			printf("0x%02X ", buffer2[i]);
		}
	}
	
	//测试4： 获取驱动缓冲区已使用的大小（抽象成文件大小）
	printf("\nused: %d\n", GetFileSize(handle, NULL));
 
	getchar();
	CloseHandle(handle);
 
	/*handle = CreateFileA("\\\\.\\MyDevice1_link", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE){
		MessageBoxA(0, "打开设备失败", "错误", 0);
		return 0;
	}
	printf("\nused: %d\n", GetFileSize(handle, NULL));
	getchar();
	CloseHandle(handle);*/
 
	return 0;
}
```

驱动程序源码：

```C++
#include <ntddk.h>
extern "C" VOID DriverUnload(PDRIVER_OBJECT pDriverObject);
extern "C" NTSTATUS DefDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp);
extern "C" NTSTATUS WriteDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp);
extern "C" NTSTATUS ReadDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp);
extern "C" NTSTATUS QueryInfomationDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp);
 
#define BUFFER_LENGTH 512 //缓冲区长度
 
//我们定义的设备扩展
typedef struct _DEVICE_EXTENSION {
	UNICODE_STRING SymLinkName;//符号链接名
 
	//这是为我们要处理读写请求而准备的缓冲区长度和指针
	ULONG filelength;//已经使用的长度（这个很像一个文件，故这样命名）
	PUCHAR buffer;//缓冲区指针
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;
 
#pragma code_seg("INIT")
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
	DbgPrint("DriverEntry\r\n");
 
	pDriverObject->DriverUnload = DriverUnload;//注册驱动卸载函数
 
	//注册派遣函数
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DefDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = WriteDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = ReadDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = QueryInfomationDispatchRoutine;
 
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;
 
	//创建设备名称的字符串
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDevice1");
 
	//创建设备
	status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION), &devName, FILE_DEVICE_UNKNOWN, 0, TRUE, &pDevObj);
	if (!NT_SUCCESS(status))
		return status;
 
	pDevObj->Flags |= DO_DIRECT_IO;//将设备设置为直接I/O设备
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到设备扩展
 
	//分配用于处理读写请求的缓冲区
	pDevExt->buffer = (PUCHAR)ExAllocatePool(PagedPool, BUFFER_LENGTH);
	//设置缓冲区已使用的大小
	pDevExt->filelength = 0;
 
	//内存清零
	RtlZeroMemory(pDevExt->buffer, BUFFER_LENGTH);
 
	//创建符号链接
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\MyDevice1_link");
	pDevExt->SymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(pDevObj);
		return status;
	}
	return STATUS_SUCCESS;
}
 
extern "C" VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	DbgPrint("DriverUnload\r\n");
	PDEVICE_OBJECT pDevObj;
	pDevObj = pDriverObject->DeviceObject;
 
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到设备扩展
 
	//删除符号链接
	UNICODE_STRING pLinkName = pDevExt->SymLinkName;
	IoDeleteSymbolicLink(&pLinkName);
 
	//删除设备
	IoDeleteDevice(pDevObj);
}
 
extern "C" NTSTATUS DefDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("DefDispatchRoutine\r\n");
	NTSTATUS status = STATUS_SUCCESS;
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return status;
}
extern "C" NTSTATUS WriteDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("WriteDispatchRoutine\r\n");
	NTSTATUS status = STATUS_SUCCESS;
 
	//得到设备扩展
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
 
	//得到I/O堆栈的当前这一层，也就是IO_STACK_LOCATION结构的指针
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
 
	ULONG WriteLength = stack->Parameters.Write.Length;//获取写入的长度
	ULONG WriteOffset = (ULONG)stack->Parameters.Write.ByteOffset.QuadPart;//获取写入的偏移量
	DbgPrint("WriteLength: %d\r\nWriteOffset: %d\r\n", WriteLength, WriteOffset);//输出相关信息
 
	if (pIrp->MdlAddress == NULL){
		//MdlAddress不能为NULL，否则下面的操作会蓝屏，因此直接失败完成
		pIrp->IoStatus.Information = 0;
		status = STATUS_UNSUCCESSFUL;
		DbgPrint("E: pIrp->MdlAddress == NULL\r\n");
	}
	else{
		//这三行代码仅用于测试MDL
		DbgPrint("MmGetMdlVirtualAddress :0x%08X\r\n", MmGetMdlVirtualAddress(pIrp->MdlAddress));
		DbgPrint("MmGetMdlByteCount: %d\r\n", MmGetMdlByteCount(pIrp->MdlAddress));
		DbgPrint("MmGetMdlByteOffset: %d\r\n", MmGetMdlByteOffset(pIrp->MdlAddress));
 
		if (MmGetMdlByteCount(pIrp->MdlAddress) != WriteLength)
		{
			//MDL中记录的长度应该和读/写长度相等，否则则是发生了错误，失败完成IRP
			pIrp->IoStatus.Information = 0;
			status = STATUS_UNSUCCESSFUL;
			DbgPrint("E: MmGetMdlByteCount(pIrp->MdlAddress) != WriteLength\r\n");
		}
		else{
			//用MmGetSystemAddressForMdlSafe得到应用程序提供的用户模式缓冲区映射到的物理内存在内核模式下的重新映射的地址
			PVOID buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
			if (buffer == NULL){
				//内核模式下的重新映射的地址不能是NULL，不然你想想把数据写到0x0位置的后果吧
				//再说MmGetSystemAddressForMdlSafe返回0表示失败，说明系统的资源不足，因此直接失败完成
				pIrp->IoStatus.Information = 0;
				status = STATUS_UNSUCCESSFUL;
				DbgPrint("E: MmGetSystemAddressForMdlSafe Failed\r\n");
			}
			else{
				DbgPrint("MmGetSystemAddressForMdlSafe :0x%08X\r\n", buffer);
				if (WriteOffset + WriteLength > BUFFER_LENGTH){
					//如果要操作的超出了缓冲区，则失败完成IRP，返回无效
					DbgPrint("E: The size of the data is too long.\r\n");
					status = STATUS_FILE_INVALID;
					pIrp->IoStatus.Information = 0;
				}
				else{
					//没有超出,则进行内存复制，将写入的数据复制缓冲区
					RtlMoveMemory(pDevExt->buffer + WriteOffset, buffer, WriteLength);
					status = STATUS_SUCCESS;
					pIrp->IoStatus.Information = WriteLength;//设置操作字节数
 
					//设置新的已经使用长度
					if (WriteLength + WriteOffset > pDevExt->filelength){
						pDevExt->filelength = WriteLength + WriteOffset;
					}
				}
			}
		}
	}
	
	pIrp->IoStatus.Status = status;//设置IRP完成状态，会设置用户模式下的GetLastError
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);//完成IRP
	return status;
}
extern "C" NTSTATUS ReadDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("ReadDispatchRoutine\r\n");
	NTSTATUS status = STATUS_SUCCESS;
 
	//得到设备扩展
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
 
	//得到I/O堆栈的当前这一层，也就是IO_STACK_LOCATION结构的指针
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
 
	ULONG ReadLength = stack->Parameters.Read.Length;//得到读的长度
	ULONG ReadOffset = (ULONG)stack->Parameters.Read.ByteOffset.QuadPart;//得到读偏移量
	DbgPrint("ReadLength: %d\r\nReadOffset: %d\r\n", ReadLength, ReadOffset);//输出相关信息
 
	if (pIrp->MdlAddress == NULL){
		//MdlAddress不能为NULL，否则下面的操作会蓝屏，因此直接失败完成
		pIrp->IoStatus.Information = 0;
		status = STATUS_UNSUCCESSFUL;
		DbgPrint("E: pIrp->MdlAddress == NULL\r\n");
	}
	else{
		//这三行代码仅用于测试MDL
		DbgPrint("MmGetMdlVirtualAddress :0x%08X\r\n", MmGetMdlVirtualAddress(pIrp->MdlAddress));
		DbgPrint("MmGetMdlByteCount: %d\r\n", MmGetMdlByteCount(pIrp->MdlAddress));
		DbgPrint("MmGetMdlByteOffset: %d\r\n", MmGetMdlByteOffset(pIrp->MdlAddress));
 
		if (MmGetMdlByteCount(pIrp->MdlAddress) != ReadLength)
		{
			//MDL中记录的长度应该和读/写长度相等，否则则是发生了错误，失败完成IRP
			pIrp->IoStatus.Information = 0;
			status = STATUS_UNSUCCESSFUL;
			DbgPrint("E: MmGetMdlByteCount(pIrp->MdlAddress) != ReadLength\r\n");
		}
		else{
			//用MmGetSystemAddressForMdlSafe得到应用程序提供的用户模式缓冲区映射到的物理内存在内核模式下的重新映射的地址
			PVOID buffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
			if (buffer == NULL){
				//内核模式下的重新映射的地址不能是NULL，从0x0位置读也肯定不对，还有可能蓝屏
				//再说MmGetSystemAddressForMdlSafe返回0表示失败，说明系统的资源不足，因此直接失败完成
				pIrp->IoStatus.Information = 0;
				status = STATUS_UNSUCCESSFUL;
				DbgPrint("E: MmGetSystemAddressForMdlSafe Failed\r\n");
			}
			else{
				DbgPrint("MmGetSystemAddressForMdlSafe :0x%08X\r\n", buffer);
				if (ReadOffset + ReadLength > BUFFER_LENGTH){
					//如果要操作的超出了缓冲区，则失败完成IRP，返回无效
					DbgPrint("E: The size of the data is too long.\r\n");
					status = STATUS_FILE_INVALID;//会设置用户模式下的GetLastError
					pIrp->IoStatus.Information = 0;
				}
				else{
					//没有超出,则进行内存复制
					RtlMoveMemory(buffer, pDevExt->buffer + ReadOffset, ReadLength);
					status = STATUS_SUCCESS;
					pIrp->IoStatus.Information = ReadLength;//设置操作字节数
				}
			}
		}
	}
 
	pIrp->IoStatus.Status = status;//设置IRP完成状态，会设置用户模式下的GetLastError
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);//完成IRP
	return status;
}
extern "C" NTSTATUS QueryInfomationDispatchRoutine(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	DbgPrint("QueryInfomationDispatchRoutine\r\n");
	//用于处理应用程序GetFileSize获取文件大小（已经使用的大小）
 
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);//得到设备扩展
	PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;//得到I/O堆栈的当前这一层，也就是IO_STACK_LOCATION结构的指针
 
	FILE_INFORMATION_CLASS fic = stack->Parameters.QueryFile.FileInformationClass;//得到FileInformationClass枚举类型
	if (fic == FileStandardInformation){
		PFILE_STANDARD_INFORMATION FileStandardInfo = (PFILE_STANDARD_INFORMATION)pIrp->AssociatedIrp.SystemBuffer;//得到缓冲区指针
		FileStandardInfo->EndOfFile = RtlConvertLongToLargeInteger(pDevExt->filelength);//设置文件大小（已经使用的大小）
	}
 
	pIrp->IoStatus.Status = STATUS_SUCCESS;//设置IRP完成状态，会设置用户模式下的GetLastError
	pIrp->IoStatus.Information = stack->Parameters.QueryFile.Length;//设置操作字节数
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);//完成IRP
	return STATUS_SUCCESS;
}

```

我们来看看效果图：


可以看出，我们的驱动程序使用直接I/O处理了来自应用程序的读/写这两个I/O请求，对于读请求，就从内存中读出数据给应用程序，对于写请求，就写进内存。代码中带了详细的注释，再结合上一篇博文，想必大家都能很轻松地看懂。另外，通过对照那些地址，想必能让大家对内存描述符表和那几个访问MDL的宏能够深刻理解了，以及使用直接I/O来提升驱动程序处理I/O的性能。﻿﻿

当应用程序打开一个设备并发出一个 I/O 请求时，会载人到驱动程序的派遣函数中，虽然 此时驱动程序的 Dispatch Function 运行在调用 I/O 函数的用户模式线程上下文中，此时访问用户模式虚拟内存并不会带来影响，但现实情况总是要比理想环境复杂很多，首先，驱动程序可能异步处理 I/O 请求（异步完成 IRP 会在以后说）驱动程序会立即返回 Dispatch Function ，将 IRP 加入一个处理队列，并在之后在其他线程中完成他，此时线程上下文可能会随时被切换，从而使用户模式的虚拟内存地址变成野指针；另一种常见的情况是处理 I/O 请求的设备并非只有一个，比如文件系统驱动程序会将 IRP 发送到磁盘设备驱动程序中，而 对于之后的设备的派遣函数而言，并不能确定运行在原来用户模式线程上下文中，这会带来同样的问题，即此时线程上下文可能会随时被切换，从而使用户模式的虚拟内存地址变成野指针。












