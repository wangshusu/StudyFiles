### bat脚本编写

> 需要使用一个bat脚本来调用 SetEnv.Cmd 来使用namke编译程序 
>
> 在总共的bat脚本中  分为两个部分  一个是start.bat启动 负责生成代码 一个是clean.bat
>
> 这里的start.bat还要负责与makefile进行传递参数



#### 1.star.bat

```bash
@echo off

::设置copy的输出目录
set "target_folder=./start"

::copy SetEnv.cmd
call :CopyFile "SetEnv.Cmd" "./dll/"
call :CopyFile "SetEnv.Cmd" "./exe/"
call :CopyFile "SetEnv.Cmd" "./start/"

::Compile dll
call :Compile "./dll" "/XP64" "nmake" "PROJ=dll32"
call :Compile "./dll" "/XP32" "nmake" "PROJ=dll64"

::Compile exe
call :Compile "./exe" "/XP64" "nmake" "PROJ=exe32"
call :Compile "./exe" "/XP32" "nmake" "PROJ=exe64"

::copy dll
call :FilterCopyFiles ./dll "dll32.dll dll64.dll"

::copy exe
call :FilterCopyFiles ./exe "exe32.exe exe64.exe"

::Complie 32-bit start func
call :Compile "./start" "/XP32" "nmake"

echo compile ok !
pause

::复制文件
:CopyFile
	set "source_folder=%~1"
	set "files_to_copy=%~2"
	copy "%source_folder%" "%files_to_copy%\"
goto:eof

::筛选复制文件
:FilterCopyFiles
	set "source_folder=%~1"
	set "files_to_copy=%~2"
	for /r "%source_folder%" %%f in (%files_to_copy%) do (
		xcopy /Y  "%%f" "%target_folder%\"
	)
goto:eof

::编译文件
:Compile
	set "arch=%~1"
	set "compiler=%~2"
	set "makefile=%~3"
	set "makefile1=%~4"
	cd ./"%arch%"
	start /wait cmd /c SetEnv.Cmd "%compiler%" "%makefile%" "%makefile1%"
	cd ..
goto:eof
```

>首行 @echo off 的作用

1.首先 @ 符号的作用是不显示本行的命令

也就是不显示  ` echo off`  这行命令

2.echo off

作用：从这行开始的下面每一行都不显示命令

> bash 函数

1.bat函数写法
如下是一个最简单的函数写法；以:`func`开始以`goto:eof`结束。:`func`和`goto:eof`之间的为函数内容，例如这里的`echo this is a bat func`

```bash
:func
echo this is a bat func
goto:eof
```

2.bat函数调用
写好了bat函数之后，我们需要调用它。如下代码片段展示了如何在bat脚本中调用函数。其实很简单，就这一句`call:func`即可。

```bash
call:func
pause

:func
echo this is a bat func
goto:eof
```

[bat函数详解](https://blog.csdn.net/peng_cao/article/details/73999076)

这里需要注意的是  `goto:eof` 中 `:`  两边并没有空格

而且函数的开头是按照 `:` 开始

函数写在下方 而不是执行语句（pause）的上方

[批处理脚本](https://blog.csdn.net/qq_15855921/article/details/126991577)

[windows脚本学习](https://blog.csdn.net/qq_26226375/article/details/122882619)

> for 循环

1.for、in和do是for语句的关键字，它们三个缺一不可；

2.%%I是for语句中对形式变量的引用，即使变量l在do后的语句中没有参与语句的执行，也是必须出现的；

3.in之后，do之前的括号不能省略；

4.command1表示字符串或变量，command2表示字符串、变量或命令语句；

意思为：循环调用command1中的元素作为参数，来参与command2的执行

此外需要注意几点

1.for语句的形式变量I，可以换成26个字母中的任意一个，这些字母会区分大小写，也就是说，%%I和%%i会被认为不是同一个变量；形式变量I还可以换成其他的字符，但是，为了不与批处理中的%0～%9这10个形式变量发生冲突，请不要随意把%%I替换为%%0～%%9中的任意一个；

2.in和do之间的command1表示的字符串或变量可以是一个，也可以是多个，每一个字符串或变量，我们称之为一个元素，每个元素之间，用空格键、跳格键、逗号、分号或等号分隔；

3.for语句依次提取command1中的每一个元素，把它的值赋予形式变量I，带到do后的command2中参与命令的执行；并且每次只提取一个元素，然后执行一次do后的命令语句，而无论这个元素是否被带到command2中参与了command2的运行；当执行完一次do后的语句之后，再提取command1中的下一个元素，再执行一次command2，如此循环，直到command1中的所有元素都已经被提取完毕，该for语句才宣告执行结束。



#### 2.clean.bat

```bash
@echo off

::clean dll
call :Cleanfile "./dll" "/XP32" "nmake" "clean"
call :Cleanfile "./dll" "/XP64" "nmake" "clean"

::clean exe
call :Cleanfile "./exe" "/XP32" "nmake" "clean"
call :Cleanfile "./exe" "/XP64" "nmake" "clean"

::clean start
call :Cleanfile "./start" "/XP64" "nmake" "clean"

::清理文件
:Cleanfile
	set "arch=%~1"
	set "compiler=%~2"
	set "makefile=%~3"
	set "makefile1=%~4"
	cd ./"%arch%"
	start /wait cmd /c SetEnv.Cmd "%compiler%" "%makefile%" "%makefile1%"
	del *.dll *.exe
	cd ..
goto:eof
```



