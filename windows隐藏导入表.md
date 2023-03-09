#### windows隐藏导入表

1.CFF explorer 可以查看倒入表

2.如何隐藏导入表(也就是windows api如何通过动态库来调用函数)

当你在Windows API中需要使用动态库时，你需要在代码中加载并链接相应的库文件。下面是一个使用动态库获取`GetFileAttributesA`函数的示例：

```c++
#include <windows.h>
#include <iostream>

int main()
{
    // Load the kernel32 library
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    if (hKernel32 == nullptr)
    {
        std::cerr << "Failed to load kernel32.dll" << std::endl;
        return 1;
    }

    // Get the address of GetFileAttributesA
    FARPROC pGetFileAttributesA = GetProcAddress(hKernel32, "GetFileAttributesA");
    if (pGetFileAttributesA == nullptr)
    {
        std::cerr << "Failed to get address of GetFileAttributesA" << std::endl;
        FreeLibrary(hKernel32);
        return 1;
    }

    // Call the function
    DWORD attributes = (*reinterpret_cast<LPFN_GETFILEATTRIBUTESA>(pGetFileAttributesA))("C:\\file.txt");
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        std::cerr << "Failed to get attributes of file" << std::endl;
        FreeLibrary(hKernel32);
        return 1;
    }

    // Print the attributes
    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        std::cout << "The file is a directory" << std::endl;
    }
    else
    {
        std::cout << "The file is not a directory" << std::endl;
    }

    // Unload the library
    FreeLibrary(hKernel32);

    return 0;
}
```

在此示例中，我们首先使用`LoadLibraryA`函数加载`kernel32.dll`库文件。然后，我们使用`GetProcAddress`函数获取`GetFileAttributesA`函数的地址。在获取函数地址后，我们可以使用它来调用函数，并传递文件路径作为参数。如果函数成功返回文件属性，则我们可以使用返回值来确定文件是否为目录。

最后，我们使用`FreeLibrary`函数卸载`kernel32.dll`库文件。

