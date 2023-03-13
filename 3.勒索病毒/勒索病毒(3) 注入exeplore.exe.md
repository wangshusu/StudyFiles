### 勒索病毒(3) 注入exeplore.exe

> 这里讲解其中用到的一些函数

#### 1.查找出exeplore.exe相符的进程ID 并获取当前进程的工作路径

>  这里有两种方式 一种是枚举所有的名字 还有一种找出一个名字相同的就行

##### 1.1 FindWindowA

`FindWindowA` 函数用于查找指定类名或窗口名的顶层窗口。该函数的原型如下：

```c++
HWND FindWindowA(
  LPCSTR lpClassName,
  LPCSTR lpWindowName
);
```

函数作用：在窗口类名和窗口标题匹配的窗口列表中查找**第一个**符合条件的顶级窗口。

参数说明：

- lpClassName：指向窗口类名的字符串，如果为 NULL，则匹配所有类名。
- lpWindowName：指向窗口标题的字符串，如果为 NULL，则匹配所有标题。

返回值：如果找到符合条件的窗口，则返回窗口句柄；否则返回 NULL。



##### 1.2 GetWindowThreadProcessId

函数原型：

```c++
DWORD GetWindowThreadProcessId(
  HWND    hWnd,
  LPDWORD lpdwProcessId
);
```

函数作用：获取指定窗口所属进程的标识符和线程标识符。

参数说明：

- hWnd：指定窗口的句柄。
- lpdwProcessId：指向保存进程标识符的变量地址。

返回值：返回指定窗口所属进程的主线程标识符。



##### 1.3 OpenProcess

函数原型：

```
c++Copy codeHANDLE OpenProcess(
  DWORD dwDesiredAccess,
  BOOL  bInheritHandle,
  DWORD dwProcessId
);
```

函数作用：打开一个**已存在**的进程，并返回该进程的句柄。

参数说明：

- dwDesiredAccess：指定进程的访问权限。
- bInheritHandle：指定新进程是否继承当前进程的句柄。
- dwProcessId：指定进程标识符。

其中，`dwDesiredAccess` 参数指定所需访问权限的标志。常用的权限包括：

- `PROCESS_CREATE_THREAD`：允许创建一个线程。
- `PROCESS_QUERY_INFORMATION`：允许获取进程信息。
- `PROCESS_VM_OPERATION`：允许对进程的虚拟内存进行操作。
- `PROCESS_VM_WRITE`：允许写入进程的虚拟内存。
- `PROCESS_VM_READ`：允许读取进程的虚拟内存。

`bInheritHandle` 参数指定新句柄是否可被子进程继承，`dwProcessId` 参数指定要打开的进程的标识符。如果函数成功，则返回值为进程句柄，否则返回 `NULL`。

返回值：如果函数执行成功，则返回进程的句柄；否则返回 NULL。



##### 1.4 GetModuleFileNameEx

函数原型：

```c++
DWORD GetModuleFileNameEx(
  HANDLE  hProcess,
  HMODULE hModule,
  LPSTR   lpFilename,
  DWORD   nSize
);
```

函数作用：获取指定进程中指定模块的完整路径。

参数说明：

- hProcess：指定进程的句柄。
- hModule：指定模块的句柄，如果为 NULL，则获取进程的主模块路径。
- lpFilename：指向存储路径的字符串缓冲区。
- nSize：缓冲区的大小。

返回值：返回复制到缓冲区的字符数，不包括 NULL 终止符；如果函数执行失败，则返回 0。



#### 2.判断要加载的dll是否存在

##### 2.1 GetFileAttributesA

函数原型如下：

```C++
DWORD GetFileAttributesA(
  LPCSTR lpFileName
);
```

函数参数说明：

- lpFileName：指向以NULL结尾的字符串，该字符串是要检索属性的文件或目录的名称。

函数返回值说明：

- 如果函数调用成功，则返回文件或目录的属性，属性值以位域形式返回。如果函数调用失败，则返回INVALID_FILE_ATTRIBUTES。

GetFileAttributesA的返回值表示了一个文件或目录的属性信息，可以使用位运算操作获取其中的具体信息。其中包括：

- FILE_ATTRIBUTE_ARCHIVE：文件或目录存档属性，即文件或目录需要归档。
- FILE_ATTRIBUTE_COMPRESSED：文件或目录压缩属性，即文件或目录已被压缩。
- FILE_ATTRIBUTE_DIRECTORY：文件或目录目录属性，即文件或目录是一个目录。
- FILE_ATTRIBUTE_HIDDEN：文件或目录隐藏属性，即文件或目录是隐藏的。
- FILE_ATTRIBUTE_NORMAL：文件或目录普通属性，即文件或目录没有其他属性。
- FILE_ATTRIBUTE_OFFLINE：文件或目录离线属性，即文件或目录处于离线状态。
- FILE_ATTRIBUTE_READONLY：文件或目录只读属性，即文件或目录只能读取，不能修改或删除。
- FILE_ATTRIBUTE_SYSTEM：文件或目录系统属性，即文件或目录是操作系统的一部分。
- FILE_ATTRIBUTE_TEMPORARY：文件或目录临时属性，即文件或目录是临时文件或目录。

> 拓展 : 

##### 2.2 DeleteFileA

函数原型如下：

```C++
BOOL DeleteFileA(
  LPCSTR lpFileName
);
```

函数参数说明：

- lpFileName：指向以NULL结尾的字符串，该字符串是要删除的文件的名称。

函数返回值说明：

- 如果函数调用成功，则返回值为非零值。如果函数调用失败，则返回值为零。

DeleteFileA函数被用来删除指定的文件。这个函数的实现非常简单，只需将文件路径传递给函数，并在返回TRUE之前等待文件删除操作完成。

当文件被删除时，文件系统将把文件从磁盘上删除。如果文件是只读的或正在被其他程序使用，那么DeleteFileA函数将不能删除该文件，并返回一个错误代码。如果指定的文件不存在，函数将返回FALSE。

需要注意的是，删除文件时要小心，因为删除文件后无法恢复。因此，在调用DeleteFileA函数之前，需要确保你已经备份了需要保留的文件或数据。此外，还需要注意文件权限，确保当前用户拥有删除指定文件的权限。



#### 3.判断当前的系统是32位还是64位(可以使用宏定义)





#### 4.判断dll是否已经被加载

##### 4.1 CreateToolhelp32Snapshot

函数原型如下：

```C++
HANDLE CreateToolhelp32Snapshot(
  DWORD dwFlags,
  DWORD th32ProcessID
);
```

函数参数说明：

- dwFlags：指定要获取的快照类型。常用的快照类型有TH32CS_SNAPPROCESS和TH32CS_SNAPMODULE。
- th32ProcessID：指定要获取快照的进程ID，如果为0，则表示获取系统中所有进程的快照。

函数返回值说明：

- 如果函数调用成功，则返回一个句柄，该句柄可以用于下一步操作。如果函数调用失败，则返回INVALID_HANDLE_VALUE。

CreateToolhelp32Snapshot函数用于创建一个快照，以便在下一步操作中使用。快照是进程、线程和模块等系统资源的静态映像，可以被用于枚举这些资源。可以通过指定不同的标志来控制快照的类型。

##### 4.2 Module32First

函数原型如下：

```C++
BOOL Module32First(
  HANDLE hSnapshot,
  LPMODULEENTRY32 lpme
);
```

函数参数说明：

- hSnapshot：由CreateToolhelp32Snapshot函数返回的快照句柄。
- lpme：指向一个MODULEENTRY32结构的指针，用于接收第一个模块的信息。

函数返回值说明：

- 如果函数调用成功，则返回一个非零值。如果函数调用失败，则返回零。

Module32First函数用于获取系统中第一个模块的信息。它需要一个快照句柄和一个指向MODULEENTRY32结构的指针。函数调用成功后，可以通过检查MODULEENTRY32结构来获取模块的信息。

##### 4.3 Module32Next

函数原型如下：

```C++
BOOL Module32Next(
  HANDLE hSnapshot,
  LPMODULEENTRY32 lpme
);
```

函数参数说明：

- hSnapshot：由CreateToolhelp32Snapshot函数返回的快照句柄。
- lpme：指向一个MODULEENTRY32结构的指针，用于接收下一个模块的信息。

函数返回值说明：

- 如果函数调用成功，则返回一个非零值。如果函数调用失败，则返回零。

Module32Next函数用于获取系统中下一个模块的信息。它需要一个快照句柄和一个指向MODULEENTRY32结构的指针。函数调用成功后，可以通过检查MODULEENTRY32结构来获取模块的信息。

需要注意的是，在调用CreateToolhelp32Snapshot函数之前，需要先为MODULEENTRY32结构中的dwSize字段赋值，以便系统知道该结构的大小。在调用Module32First和Module32Next函数之前，需要检查它们的返回值，确保操作成功。此外，在使用完快照之后，需要调用CloseHandle函数来关闭句

> 例子如下 :

```C++
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

// 判断指定的DLL是否已经被加载到指定进程中
BOOL IsModuleLoaded(DWORD dwPID, LPCTSTR lpModuleName)
{
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32 = {0};
    BOOL bFound = FALSE;

    // Take a snapshot of all modules in the specified process.
    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    // Set the size of the structure before using it.
    me32.dwSize = sizeof(MODULEENTRY32);

    // Retrieve information about the first module,
    // and exit if unsuccessful.
    if (!Module32First(hModuleSnap, &me32))
    {
        CloseHandle(hModuleSnap);
        return FALSE;
    }

    // Now walk the module list of the process,
    // and display information about each module
    // until there are no more modules or we find the
    // specified module.
    do
    {
        if (_tcsicmp(me32.szModule, lpModuleName) == 0)
        {
            bFound = TRUE;
            break;
        }
    } while (Module32Next(hModuleSnap, &me32));

    CloseHandle(hModuleSnap);

    return bFound;
}
```



#### 5.加载dll文件

> 1.打开获取到的进程ID(OpenProcess)
>
> 2.

##### 5.1 OpenProcess

函数原型：

```
HANDLE OpenProcess(
  DWORD dwDesiredAccess,  // 访问权限
  BOOL bInheritHandle,    // 是否继承句柄
  DWORD dwProcessId       // 进程ID
);
```

作用:

`OpenProcess`: 打开一个已存在的进程，并返回一个进程句柄。可以用该句柄进行进程相关的操作，如读写内存、挂起/恢复进程等。该函数的作用是获取目标进程的进程句柄。

参数说明：

- `dwDesiredAccess`：指定打开进程的访问权限，具体取值参考MSDN文档。例如，如果需要读取远程进程中的内存，应该指定`PROCESS_VM_READ`权限。
- `bInheritHandle`：是否允许在子进程中继承该进程句柄。如果需要在创建子进程时继承该句柄，则应该将该参数设置为`TRUE`，否则应该将其设置为`FALSE`。
- `dwProcessId`：进程ID。

返回值说明：

- 成功：返回打开的进程句柄。
- 失败：返回NULL。可使用`GetLastError()`获取详细错误信息。



##### 5.2 VirtualAllocEx

1. 函数原型：

```
LPVOID VirtualAllocEx(
  HANDLE hProcess,       // 进程句柄
  LPVOID lpAddress,      // 指定内存地址，可选
  SIZE_T dwSize,         // 分配的内存大小
  DWORD flAllocationType,// 内存分配类型
  DWORD flProtect        // 内存保护类型
);
```

作用:

`VirtualAllocEx`: 在指定进程的虚拟地址空间中分配内存。该函数的作用是在目标进程中分配内存，以便后续的写入操作。

参数说明：

- `hProcess`：进程句柄。
- `lpAddress`：分配内存的起始地址，若指定为NULL，则由系统自动分配地址。
- `dwSize`：分配的内存大小，单位为字节。
- `flAllocationType`：内存分配类型，取值范围参考MSDN文档。
- `flProtect`：内存保护类型，取值范围参考MSDN文档。

返回值说明：

- 成功：返回分配内存的起始地址。
- 失败：返回NULL。可使用`GetLastError()`获取详细错误信息。



##### 5.3 WriteProcessMemory

1. 函数原型：

```
cCopy codeBOOL WriteProcessMemory(
  HANDLE hProcess,          // 进程句柄
  LPVOID lpBaseAddress,     // 写入内存的起始地址
  LPCVOID lpBuffer,         // 需要写入的数据缓冲区
  SIZE_T nSize,             // 写入的数据大小，单位为字节
  SIZE_T *lpNumberOfBytesWritten // 实际写入的数据大小
);
```

作用:

`WriteProcessMemory`: 将数据写入到指定进程的内存中。该函数的作用是将数据写入到已分配的目标进程内存中，以便后续的远程线程执行。

参数说明：

- `hProcess`：进程句柄。
- `lpBaseAddress`：写入内存的起始地址。
- `lpBuffer`：需要写入的数据缓冲区。
- `nSize`：写入的数据大小，单位为字节。
- `lpNumberOfBytesWritten`：实际写入的数据大小，由函数返回。

返回值说明：

- 成功：返回TRUE。
- 失败：返回FALSE。可使用`GetLastError()`获取详细错误信息。



##### 5.4 CreateRemoteThread

函数原型：

```C++
HANDLE CreateRemoteThread(
  HANDLE                 hProcess,
  LPSECURITY_ATTRIBUTES  lpThreadAttributes,
  SIZE_T                 dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress,
  LPVOID                 lpParameter,
  DWORD                  dwCreationFlags,
  LPDWORD                lpThreadId
);
```

作用:

`CreateRemoteThread`: 在指定进程中创建一个远程线程，并返回线程句柄。远程线程在目标进程中运行，因此可以用来执行一些需要在目标进程中运行的代码，如加载DLL、执行代码等。该函数的作用是在目标进程中创建一个远程线程，并在该线程中执行指定的代码。

参数说明：

- hProcess：目标进程的句柄，可以使用`OpenProcess`获得
- lpThreadAttributes：线程安全描述，通常设置为NULL即可
- dwStackSize：新线程的堆栈大小，如果为0，则使用与创建线程的进程相同的大小
- lpStartAddress：新线程的起始地址，即线程函数的地址
- lpParameter：传递给线程函数的参数，为可选参数，可设置为NULL
- dwCreationFlags：线程的创建标志，通常设置为0即可
- lpThreadId：返回新线程的ID，通常设置为NULL即可

返回值说明：

- 如果函数执行成功，则返回新线程的句柄
- 如果函数执行失败，则返回NULL

函数作用： 在目标进程中创建一个新的线程。



##### 5.5 WaitForSingleObject

函数原型：

```C++
DWORD WaitForSingleObject(
  HANDLE hHandle,
  DWORD  dwMilliseconds
);
```

作用:

`WaitForSingleObject`: 等待指定的内核对象变为可用状态，或等待指定的时间。该函数的作用是等待远程线程执行完毕，并获取线程的返回值。

参数说明：

- hHandle：等待的对象句柄，这里一般为线程句柄或者进程句柄
- dwMilliseconds：等待时间，单位为毫秒，如果为INFINITE，则一直等待直到对象状态变为已经通知状态

返回值说明：

- 如果函数返回WAIT_OBJECT_0，则表示对象状态已经通知
- 如果函数返回WAIT_TIMEOUT，则表示等待时间超时
- 如果函数返回WAIT_FAILED，则表示函数执行失败

函数作用： 等待一个对象的状态变为已经通知状态。