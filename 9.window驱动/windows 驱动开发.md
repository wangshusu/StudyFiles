### windows 驱动开发

#### 1. 环境设置

> 环境配置

如果一大堆的头文件找不到报错可能是因为SDK与WDF不对应

[VS2017、2019驱动WDF开发环境找不到头文件](https://blog.csdn.net/weixin_45935855/article/details/127194299)



> 编译设置

 1.创建好项目工程后需要删除

![image-20230406202250239](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230406202250239.png)

2. 设置中不生成object不报错 和 错误等级三级(可以忽略形参不使用报错的bug)

![image-20230406203042779](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230406203042779.png)

3. 设置相同的编译版本

![image-20230406202957716](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230406202957716.png)



####2.  简单的 hello world 驱动

```C++
#include<ntddk.h>
#include <wdm.h>  
#include <version.h>
#include "assert.h"
#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

typedef struct _DEVICE_EXTENSION {
	PDEVICE_OBJECT pDevice;			// 指向设备对象的指针
	UNICODE_STRING ustrDeviceName;	// 设备名称字符串，使用 Unicode 编码
	UNICODE_STRING ustrSymLinkName; // 设备符号链接名称字符串，使用 Unicode 编码
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;


NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject);
VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject);
NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status;   //定义NTSTATUS类型的变量status
	KdPrint(("Enter DriverEntry\n"));  //打印调试信息

	pDriverObject->DriverUnload = HelloDDKUnload;  //将驱动卸载函数指定为HelloDDKUnload

	for (int i = 0; i < arraysize(pDriverObject->MajorFunction); ++i)  //遍历MajorFunction数组
		pDriverObject->MajorFunction[i] = HelloDDKDispatchRoutin;  //将各个IRP处理函数指定为HelloDDKDispatchRoutin

	status = CreateDevice(pDriverObject);   //创建设备对象

	KdPrint(("DriverEntry end\n"));  //打印调试信息
	return status;   //返回status变量
}

// 函数名称：NTSTATUS CreateDevice
// 输入参数：IN PDRIVER_OBJECT  pDriverObject  驱动对象指针
// 返回值：NTSTATUS  函数执行状态码
// 功能描述：创建设备对象并将其挂接到驱动程序中
NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject) {
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;
	PDEVICE_EXTENSION pDevExt;

	// 设备名称字符串
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyDDKDevice");

	// 创建设备对象
	status = IoCreateDevice(pDriverObject,        // 驱动对象指针
		sizeof(DEVICE_EXTENSION),    // 设备扩展结构体的大小
		&devName,           // 设备名称
		FILE_DEVICE_UNKNOWN,        // 设备类型
		0, TRUE,            // 设备特性
		&pDevObj);          // 指向设备对象指针的指针
	if (!NT_SUCCESS(status)) 
	{
		// 创建设备对象失败
		KdPrint(("create Device faild1 %s \n"));
		return status;
	}

	// 设备扩展结构体
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;
	pDevExt->pDevice = pDevObj;             // 保存设备对象指针
	pDevExt->ustrDeviceName = devName;      // 保存设备名称字符串

											// 符号链接名称字符串
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\WorkItem");
	pDevExt->ustrSymLinkName = symLinkName;

	// 创建符号链接
	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		// 创建符号链接失败，删除设备对象
		IoDeleteDevice(pDevObj);
		KdPrint(("create Device faild 2%s \n"));
		return status;
	}

	// 返回函数执行成功状态码
	return STATUS_SUCCESS;
}


VOID HelloDDKUnload(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT pNextObj;

	KdPrint(("Enter DriverUnload\n"));

	pNextObj = pDriverObject->DeviceObject; // 获取第一个设备对象的指针

	while (pNextObj != NULL) // 遍历设备对象链表，逐个删除设备对象
	{
		
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pNextObj->DeviceExtension; // 获取设备扩展结构体的指针

		UNICODE_STRING pLinkName = pDevExt->ustrSymLinkName;
		IoDeleteSymbolicLink(&pLinkName); // 删除符号链接

		// 获取下一个设备对象的指针
		pNextObj = pNextObj->NextDevice;

		// 删除设备对象并释放相关资源
		IoDeleteDevice(pDevExt->pDevice);
	}
}

//处理IRP请求
NTSTATUS HelloDDKDispatchRoutin(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	// 获取当前IRP的堆栈位置信息
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);

	// 定义IRP请求类型名称的字符串数组
	static char* irpname[] =
	{
		"IRP_MJ_CREATE",
		"IRP_MJ_CREATE_NAMED_PIPE",
		"IRP_MJ_CLOSE",
		"IRP_MJ_READ",
		"IRP_MJ_WRITE",
		"IRP_MJ_QUERY_INFORMATION",
		"IRP_MJ_SET_INFORMATION",
		"IRP_MJ_QUERY_EA",
		"IRP_MJ_SET_EA",
		"IRP_MJ_FLUSH_BUFFERS",
		"IRP_MJ_QUERY_VOLUME_INFORMATION",
		"IRP_MJ_SET_VOLUME_INFORMATION",
		"IRP_MJ_DIRECTORY_CONTROL",
		"IRP_MJ_FILE_SYSTEM_CONTROL",
		"IRP_MJ_DEVICE_CONTROL",
		"IRP_MJ_INTERNAL_DEVICE_CONTROL",
		"IRP_MJ_SHUTDOWN",
		"IRP_MJ_LOCK_CONTROL",
		"IRP_MJ_CLEANUP",
		"IRP_MJ_CREATE_MAILSLOT",
		"IRP_MJ_QUERY_SECURITY",
		"IRP_MJ_SET_SECURITY",
		"IRP_MJ_POWER",
		"IRP_MJ_SYSTEM_CONTROL",
		"IRP_MJ_DEVICE_CHANGE",
		"IRP_MJ_QUERY_QUOTA",
		"IRP_MJ_SET_QUOTA",
		"IRP_MJ_PNP",
	};

	// 获取当前IRP请求的类型
	UCHAR type = stack->MajorFunction;
	if (type >= arraysize(irpname))
		// 未知的IRP类型
		KdPrint((" - Unknown IRP, major type %X\n", type));
	else
		// 打印当前IRP请求类型的名称
		KdPrint(("\t%s\n", irpname[type]));

	// 初始化返回状态为成功
	NTSTATUS status = STATUS_SUCCESS;

	// 将IRP请求状态设置为成功，并设置传输的字节数
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered

									// 将IRP请求完成，并释放IRP的资源
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	// 打印调试信息
	KdPrint(("Leave HelloDDKDispatchRoutin\n"));

	// 返回请求的状态
	return status;
}
```











#### 3. 编译其它平台的驱动

1. 设置需要编译的版本

![image-20230406155636685](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230406155636685.png)

2.修改参数为off

![image-20230406155603567](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230406155603567.png)



#### 4. 将驱动加载到win系统中

1.写入注册表中

创建 ` MyDriver.reg` 文件 写入

```C
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\MyDriver4]
"DisplayName"="MyDriver4"
"ImagePath"="\\SystemRoot\\System32\\drivers\\MyDriver4.sys"
"Start"=dword:00000003
"Type"=dword:00000001
"ErrorControl"=dword:00000001
```

这个.REG文件包括以下注册表项:

1. "DisplayName"：指定服务的名称，将显示在服务控制管理器中。
2. "ImagePath"：指定驱动程序文件的路径和名称。
3. "Start"：指定服务启动类型。在这个例子中，"dword:00000003"表示服务应该手动启动。
4. "Type"：指定服务类型。在这个例子中，"dword:00000001"表示服务是内核模式驱动程序。
5. "ErrorControl"：指定服务启动时出现错误时的行为。在这个例子中，"dword:00000001"表示系统将显示一个错误对话框。

关于指定服务启动类型 ↓

在Windows系统中，服务可以设置为以下四种启动类型之一：

1. 自动启动：当系统启动时，服务将自动启动。
2. 手动启动：服务不会在系统启动时自动启动，但可以手动启动。
3. 禁用：服务被禁用，并且不能被手动或自动启动。
4. 延迟启动：服务将在系统启动后一段时间内延迟启动，以提高启动速度。



> 双击运行就可以在注册表中写入值 可以查看和修改

![image-20230406172440789](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230406172440789.png)



> sc命令
>
> 这里  只需要将服务暂停和启动即可 不需要使用删除

4.启动一个服务 
sc start 服务名 

5.暂停一个服务 
sc pause 服务名 

6.继续一个服务 
sc continue 服务名 

7.停止一个服务 
sc stop 服务名 

8.禁止一个服务 
sc 服务名 config start= disabled 

9.将一个服务设为自动运行 
sc 服务名 config start= auto 

10.将一个服务设为手动运行 
sc 服务名 config start= demand 

11.删除一个服务

sc delete 服务名

SC OpenService 失败5:拒绝访问 (管理员权限打开就好了)



[sc命令详解](https://blog.csdn.net/jiangqin115/article/details/46531099)





> 使用DebugView来查看调试信息

1.首先在注册表中写入

Dbgview.reg

```C
Windows Registry Editor Version 5.00

[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Debug Print Filter]
"DEFAULT"=dword:0000000f
```

注册表中有了之后 再使用**管理员权限**运行



[DebugView调试技巧](https://blog.csdn.net/freeking101/article/details/103812897)

[调试操作](https://www.cnblogs.com/bianchengnan/p/12243145.html)





> 使用KmdManager加载驱动



### 链接

[博客园-驱动对象设备对象的区别](https://www.cnblogs.com/lsh123/p/7455901.html)

