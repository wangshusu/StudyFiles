### 使用 SDK MakeFile 生成 dll

1. sdk

![image-20230314162650454](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230314162650454.png)

2.完成makefile的编辑后 使用sdk 建议debug版本切换到目录 使用nmake进行编译

![image-20230314162859886](https://bucketforago.oss-cn-shenzhen.aliyuncs.com/typora/image-20230314162859886.png)



### SDK Makefile

> 本次编程用到的MakeFile不算困难  下面是一些介绍 算是对它的一些了解 这里的 Makefile 一共有三种
>
> 1.编译dll库文件 2.编译exe可执行文件 3.编译带有资源的可执行文件

#### 1. exe Makefile

> 先会对其中的参数做一些简单的介绍 然后简单的概述写法

```makefile
# Nmake macros for building Windows 32-64-bits exe

!include <ntwin32.mak>

PROJ=
all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

OBJS=$(OUTDIR)\winproc.obj $(OUTDIR)\encdemo.obj  $(OUTDIR)\exe64.obj

ELIBS=psapi.lib

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the object file if necessary

$(OUTDIR)\winproc.obj: winproc.cpp winproc.h
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" winproc.cpp

$(OUTDIR)\encdemo.obj: encdemo.cpp encdemo.h
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" encdemo.cpp
	
$(OUTDIR)\exe64.obj: exe64.cpp exe64.h
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" exe64.cpp

$(OUTDIR)\$(PROJ).exe: $(OBJS)
    $(link) $(ldebug) $(guilflags) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).exe $(OBJS) $(guilibs) $(ELIBS)
	
.PHONY : clean cleant cleane
clean:
	$(CLEANUP)

cleant:
	if exist $(OUTDIR) del $(OUTDIR)\*.obj
	if exist $(OUTDIR) del $(OUTDIR)\*.pdb

cleane:
	if exist $(OUTDIR) del $(OUTDIR)\*.exe
```

1. 首先开头的 ` !include <ntwin32.mak>` 是一个预处理器指令，用于在 Microsoft Visual Studio 的 C 或 C++ 项目中包含名为 "ntwin32.mak" 的头文件。

"ntwin32.mak" 头文件包含了一些宏定义和变量声明，用于简化 Windows 应用程序的构建过程。例如，它包含了许多 Windows API 函数的定义和库文件的链接器选项，使得开发者可以更方便地使用这些函数和选项来编写 Windows 应用程序。

> 下面的库文件都是可能包含的  也是对这些库文件有一个大致的了解

- `kernel32.lib`：包含了 Windows 内核的许多系统函数，例如文件和目录操作、进程和线程操作、内存管理等等。
- `user32.lib`：包含了用户界面函数，例如窗口管理、消息处理、菜单和对话框等等。
- `gdi32.lib`：包含了图形设备接口函数，例如绘图和字体操作、打印输出等等。
- `advapi32.lib`：包含了安全和权限管理函数，例如访问控制、加密解密、身份验证等等。
- `comdlg32.lib`：包含了通用对话框函数，例如打开和保存文件对话框、字体和颜色对话框等等。



> 其中 PROJ all ELIBS 这些就不再赘述  可以通过基本的makefile来进行了解
>
> 主要介绍其中生成 obj 文件的过程  和选项

2. 根据依赖文件 .cpp .h 来生成 .obj

注意  这里的  `$(OUTDIR)  ` 是生成的目录， 具体如何生成我也不清楚，但是使用SDK 的 nmake 会自动生成  下面的 $(CC) 也是同理

>  下面给出 这一句 中的参数一些解释

```C++
$(OUTDIR)\winproc.obj: winproc.cpp winproc.h
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" winproc.cpp
```

- `$(OUTDIR)`：表示输出目录，这是 Visual Studio 中指定的输出目录，用于存放生成的可执行文件和中间文件等。
- `winproc.obj`：表示目标文件名，这里生成的目标文件名为 "winproc.obj"。
- `winproc.cpp`：表示源文件名，需要编译的源代码文件名为 "winproc.cpp"。
- `$(cc)`：表示 C++ 编译器的路径，通常为 "cl.exe"。
- `$(cdebug)`：表示编译器的调试选项，这里表示编译器开启了调试信息的生成。
- `$(cflags)`：表示编译器的编译选项，例如优化选项、警告选项等等。
- `$(cvarsmt)`：表示编译器的多线程选项，这里表示编译器使用了多线程编译。
- `/Fo"$(OUTDIR)\\`：表示编译器生成目标文件的输出路径，这里表示将目标文件输出到指定的输出目录中。
- `/Fd"$(OUTDIR)\\"`：表示编译器生成调试信息的输出路径，这里表示将调试信息输出到指定的输出目录中。

细说一下 ： 

`$(cvarsmt)` 是一个 Visual Studio 的 C/C++ 项目编译器选项，它用于设置编译器如何处理多线程相关的变量和函数。

在 Windows 系统中，每个线程都有自己的栈空间和寄存器状态，这就需要对多线程相关的变量和函数进行特殊的处理，以保证线程之间的数据安全和正确性。

`$(cvarsmt)` 是一个宏变量，其值可以是以下几种选项之一：

- `/MT`：使用静态链接的多线程 C/C++ 标准库（libcmt.lib），每个程序只有一个全局变量实例。
- `/MTd`：使用静态链接的多线程调试 C/C++ 标准库（libcmtd.lib），每个程序只有一个全局变量实例，同时包含调试信息。
- `/MD`：使用动态链接的多线程 C/C++ 标准库（msvcrt.lib），每个 DLL 和 EXE 都有自己的全局变量实例。
- `/MDd`：使用动态链接的多线程调试 C/C++ 标准库（msvcrtd.lib），每个 DLL 和 EXE 都有自己的全局变量实例，同时包含调试信息。

其中，`/MT` 和 `/MTd` 选项会将多线程支持的代码静态链接到目标文件中，而 `/MD` 和 `/MDd` 选项则会将多线程支持的代码动态链接到目标文件中。使用静态链接可以使得程序更加独立，但同时也会使得程序的大小变大。使用动态链接则可以使得程序更加灵活，但同时也会增加运行时的开销。

#### 2. dll Makefile

> 与exe的不同之处是他需要根据def(函数导出)文件生成一个 .exp 文件 后续的 exe 文件的生成需要 .exp 文件的支持
>
> 这里选择在生成 lib 文件的同时生成 .def 文件 

```makefile
# Nmake macros for building Windows 32-64-bit dll

!include <win32.mak>

PROJ=

all: $(OUTDIR) $(OUTDIR)\$(PROJ).lib $(OUTDIR)\$(PROJ).dll

OBJS=$(OUTDIR)\encdemo.obj $(OUTDIR)\winproc.obj

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the object files if necessary

$(OUTDIR)\encdemo.obj: encdemo.cpp encdemo.h
    $(cc) $(cflags) $(cvarsdll) $(cdebug)  /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" encdemo.cpp
	
$(OUTDIR)\winproc.obj: winproc.cpp encdemo.h
    $(cc) $(cflags) $(cvarsdll) $(cdebug)  /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" winproc.cpp

$(OUTDIR)\$(PROJ).lib: $(OBJS)  dll64.def
    $(implib) -machine:$(CPU) \
    -def:dll64.def \
    $(OBJS) \
     -out:$(OUTDIR)\$(PROJ).lib

$(OUTDIR)\$(PROJ).dll: $(OBJS) 
    $(link) $(linkdebug) $(dlllflags) \
    -out:$(OUTDIR)\$(PROJ).dll \
    $(OUTDIR)\$(PROJ).exp $(OBJS) $(guilibsdll)

.PHONY : clean cleant cleane
clean:
	$(CLEANUP)

cleant:
	if exist $(OUTDIR) del $(OUTDIR)\*.obj
	if exist $(OUTDIR) del $(OUTDIR)\*.pdb

cleane:
	if exist $(OUTDIR) del $(OUTDIR)\*.dll
	if exist $(OUTDIR) del $(OUTDIR)\*.lib

```



#### 3. start.exe Makefile

> start.exe 的 Makefile 最大的不同是他带有一个 res.rc 文件(资源文件) 资源文件的添加方法后序补充

```makefile
# Nmake macros for building Windows 32-Bit apps

!include <ntwin32.mak>

PROJ = start
all: $(OUTDIR) $(OUTDIR)\$(PROJ).exe

OBJS = $(OUTDIR)\start.obj \
	   $(OUTDIR)\resource.res

ELIBS =

$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

# Update the object file if necessary

$(OUTDIR)\start.obj: start.cpp start.h resource.h
    $(cc) $(cdebug) $(cflags) $(cvarsmt) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" start.cpp

$(OUTDIR)\resource.res:
     $(rc) $(rcflags) $(rcvars) /Fo$(OUTDIR)\resource.res res.rc

$(OUTDIR)\$(PROJ).exe: $(OBJS)
    $(link) $(ldebug) $(guilflags) /MACHINE:$(CPU) -out:$(OUTDIR)\$(PROJ).exe $(OBJS) $(guilibs) $(ELIBS)
	
.PHONY : clean cleant cleane
clean:
	$(CLEANUP)

cleant:
	

cleane:
```















### .rc 资源文件的创建方式

